# Copyright 2023 Ant Group Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import math
import threading
import time
from dataclasses import dataclass
from enum import Enum, unique
from typing import Dict, List, Union

import cleantext
from componentize.component.data_utils import DistDataType, check_io_def
from secretflow.spec.v1.component_pb2 import AttributeDef, AttrType, ComponentDef, IoDef
from secretflow.spec.v1.evaluation_pb2 import NodeEvalParam, NodeEvalResult


def clean_text(x: str, no_line_breaks: bool = True) -> str:
    return cleantext.clean(x.strip(), lower=False, no_line_breaks=no_line_breaks)


class CompDeclError(Exception): ...


class CompEvalError(Exception): ...


class CompTracer:
    def __init__(self) -> None:
        self.lock = threading.Lock()
        self.io_time = 0
        self.run_time = 0

    def trace_io(self):
        class _CompTracer:
            def __init__(self, tracer: CompTracer) -> None:
                self.tracer = tracer

            def __enter__(self):
                self.start_time = time.time()

            def __exit__(self, *_):
                io_time = time.time() - self.start_time
                with self.tracer.lock:
                    self.tracer.io_time += io_time

        return _CompTracer(self)

    def trace_running(self):
        class _CompTracer:
            def __init__(self, tracer: CompTracer) -> None:
                self.tracer = tracer

            def __enter__(self):
                self.start_time = time.time()

            def __exit__(self, *_):
                running_time = time.time() - self.start_time
                with self.tracer.lock:
                    self.tracer.run_time += running_time

        return _CompTracer(self)

    def report(self) -> Dict[str, float]:
        return {"io_time": self.io_time, "run_time": self.run_time}


@unique
class IoType(Enum):
    INPUT = 1
    OUTPUT = 2


@dataclass
class TableColParam:
    name: str
    desc: str
    col_min_cnt_inclusive: int = None
    col_max_cnt_inclusive: int = None


class Component:
    def __init__(self, name: str, domain="", version="", desc="") -> None:
        self.name = name
        self.domain = domain
        self.version = version
        self.desc = clean_text(desc, no_line_breaks=False)

        self.__definition = None
        self.__eval_callback = None
        self.__comp_attr_decls = []
        self.__input_io_decls = []
        self.__output_io_decls = []
        self.__argnames = set()

    def _check_reserved_words(self, word: str):
        RESERVED = ["input", "output"]
        if word in RESERVED:
            raise CompDeclError(f"{word} is a reserved word.")

    def float_attr(
        self,
        name: str,
        desc: str,
        is_list: bool,
        is_optional: bool,
        default_value: Union[List[float], float] = None,
        allowed_values: List[float] = None,
        lower_bound: float = None,
        upper_bound: float = None,
        lower_bound_inclusive: bool = False,
        upper_bound_inclusive: bool = False,
        list_min_length_inclusive: int = None,
        list_max_length_inclusive: int = None,
    ):
        # sanity checks
        self._check_reserved_words(name)

        if is_optional and default_value is None:
            raise CompDeclError(
                f"attr {name}: default_value must be provided if optional."
            )

        if allowed_values is not None and (
            lower_bound is not None or upper_bound is not None
        ):
            raise CompDeclError(
                "allowed_values and bounds could not be set at the same time."
            )

        if allowed_values is not None and default_value is not None:
            if is_list:
                for v in default_value:
                    if v not in allowed_values:
                        raise CompDeclError(
                            f"default_value {v} is not in allowed_values {allowed_values}"
                        )
            else:
                if default_value not in allowed_values:
                    raise CompDeclError(
                        f"default_value {default_value} is not in allowed_values {allowed_values}"
                    )

        if (
            lower_bound is not None
            and upper_bound is not None
            and lower_bound > upper_bound
        ):
            raise CompDeclError(
                f"lower_bound {lower_bound} is greater than upper_bound {upper_bound}"
            )

        if default_value is not None:
            if lower_bound is not None:
                if is_list:
                    for v in default_value:
                        if not (
                            v > lower_bound
                            or (lower_bound_inclusive and math.isclose(v, lower_bound))
                        ):
                            raise CompDeclError(
                                f"default_value {v} fails bound check: lower_bound {lower_bound}, lower_bound_inclusive {lower_bound_inclusive}"
                            )
                else:
                    if not (
                        default_value > lower_bound
                        or (
                            lower_bound_inclusive
                            and math.isclose(default_value, lower_bound)
                        )
                    ):
                        raise CompDeclError(
                            f"default_value {default_value} fails bound check: lower_bound {lower_bound}, lower_bound_inclusive {lower_bound_inclusive}"
                        )

        if default_value is not None:
            if upper_bound is not None:
                if is_list:
                    for v in default_value:
                        if not (
                            v < upper_bound
                            or (upper_bound_inclusive and math.isclose(v, upper_bound))
                        ):
                            raise CompDeclError(
                                f"default_value {v} fails bound check: upper_bound {upper_bound}, upper_bound_inclusive {upper_bound_inclusive}"
                            )
                else:
                    if not (
                        default_value < upper_bound
                        or (
                            upper_bound_inclusive
                            and math.isclose(default_value, upper_bound)
                        )
                    ):
                        raise CompDeclError(
                            f"default_value {default_value} fails bound check: upper_bound {upper_bound}, upper_bound_inclusive {upper_bound_inclusive}"
                        )

        if (
            list_min_length_inclusive is not None
            and list_max_length_inclusive is not None
            and list_min_length_inclusive > list_max_length_inclusive
        ):
            raise CompDeclError(
                f"list_min_length_inclusive {list_min_length_inclusive} should not be greater than list_max_length_inclusive {list_max_length_inclusive}."
            )

        # create pb
        attr = AttributeDef(
            name=name,
            desc=clean_text(desc),
            type=AttrType.AT_FLOATS if is_list else AttrType.AT_FLOAT,
            atomic=AttributeDef.AtomicAttrDesc(
                is_optional=is_optional,
            ),
        )

        if default_value is not None:
            if is_list:
                attr.atomic.default_value.fs.extend(default_value)
            else:
                attr.atomic.default_value.f = default_value

        if allowed_values is not None:
            attr.atomic.allowed_values.fs.extend(allowed_values)

        if lower_bound is not None:
            attr.atomic.has_lower_bound = True
            attr.atomic.lower_bound_inclusive = lower_bound_inclusive
            attr.atomic.lower_bound.f = lower_bound

        if upper_bound is not None:
            attr.atomic.has_upper_bound = True
            attr.atomic.upper_bound_inclusive = upper_bound_inclusive
            attr.atomic.upper_bound.f = upper_bound

        if is_list:
            if list_min_length_inclusive is not None:
                attr.atomic.list_min_length_inclusive = list_min_length_inclusive
            else:
                attr.atomic.list_min_length_inclusive = 0

            if list_max_length_inclusive is not None:
                attr.atomic.list_max_length_inclusive = list_max_length_inclusive
            else:
                attr.atomic.list_max_length_inclusive = -1

        # append
        self.__comp_attr_decls.append(attr)

    def int_attr(
        self,
        name: str,
        desc: str,
        is_list: bool,
        is_optional: bool,
        default_value: Union[List[int], int] = None,
        allowed_values: List[int] = None,
        lower_bound: int = None,
        upper_bound: int = None,
        lower_bound_inclusive: bool = False,
        upper_bound_inclusive: bool = False,
        list_min_length_inclusive: int = None,
        list_max_length_inclusive: int = None,
    ):
        # sanity checks
        self._check_reserved_words(name)

        if is_optional and default_value is None:
            raise CompDeclError(
                f"attr {name}: default_value must be provided if optional."
            )

        if allowed_values is not None and (
            lower_bound is not None or upper_bound is not None
        ):
            raise CompDeclError(
                "allowed_values and bounds could not be set at the same time."
            )

        if allowed_values is not None and default_value is not None:
            if is_list:
                for v in default_value:
                    if v not in allowed_values:
                        raise CompDeclError(
                            f"default_value {v} is not in allowed_values {allowed_values}"
                        )
            else:
                if default_value not in allowed_values:
                    raise CompDeclError(
                        f"default_value {default_value} is not in allowed_values {allowed_values}"
                    )

        if (
            lower_bound is not None
            and upper_bound is not None
            and lower_bound > upper_bound
        ):
            raise CompDeclError(
                f"lower_bound {lower_bound} is greater than upper_bound {upper_bound}"
            )

        if default_value is not None:
            if lower_bound is not None:
                if is_list:
                    for v in default_value:
                        if not (
                            v > lower_bound
                            or (lower_bound_inclusive and v == lower_bound)
                        ):
                            raise CompDeclError(
                                f"attr {name} default_value {v} fails bound check: lower_bound {lower_bound}, lower_bound_inclusive {lower_bound_inclusive}"
                            )
                else:
                    if not (
                        default_value > lower_bound
                        or (lower_bound_inclusive and default_value == lower_bound)
                    ):
                        raise CompDeclError(
                            f"attr {name} default_value {default_value} fails bound check: lower_bound {lower_bound}, lower_bound_inclusive {lower_bound_inclusive}"
                        )

        if default_value is not None:
            if upper_bound is not None:
                if is_list:
                    for v in default_value:
                        if not (
                            v < upper_bound
                            or (upper_bound_inclusive and v == upper_bound)
                        ):
                            raise CompDeclError(
                                f"attr {name} default_value {v} fails bound check: upper_bound {upper_bound}, upper_bound_inclusive {upper_bound_inclusive}"
                            )
                else:
                    if not (
                        default_value < upper_bound
                        or (upper_bound_inclusive and default_value == upper_bound)
                    ):
                        raise CompDeclError(
                            f"attr {name} default_value {default_value} fails bound check: upper_bound {upper_bound}, upper_bound_inclusive {upper_bound_inclusive}"
                        )

        if (
            list_min_length_inclusive is not None
            and list_max_length_inclusive is not None
            and list_min_length_inclusive > list_max_length_inclusive
        ):
            raise CompDeclError(
                f"list_min_length_inclusive {list_min_length_inclusive} should not be greater than list_max_length_inclusive {list_max_length_inclusive}."
            )

        # create pb
        attr = AttributeDef(
            name=name,
            desc=clean_text(desc),
            type=AttrType.AT_INTS if is_list else AttrType.AT_INT,
            atomic=AttributeDef.AtomicAttrDesc(
                is_optional=is_optional,
            ),
        )

        if default_value is not None:
            if is_list:
                attr.atomic.default_value.i64s.extend(default_value)
            else:
                attr.atomic.default_value.i64 = default_value

        if allowed_values is not None:
            attr.atomic.allowed_values.i64s.extend(allowed_values)

        if lower_bound is not None:
            attr.atomic.has_lower_bound = True
            attr.atomic.lower_bound_inclusive = lower_bound_inclusive
            attr.atomic.lower_bound.i64 = lower_bound

        if upper_bound is not None:
            attr.atomic.has_upper_bound = True
            attr.atomic.upper_bound_inclusive = upper_bound_inclusive
            attr.atomic.upper_bound.i64 = upper_bound

        if is_list:
            if list_min_length_inclusive is not None:
                attr.atomic.list_min_length_inclusive = list_min_length_inclusive
            else:
                attr.atomic.list_min_length_inclusive = 0

            if list_max_length_inclusive is not None:
                attr.atomic.list_max_length_inclusive = list_max_length_inclusive
            else:
                attr.atomic.list_max_length_inclusive = -1

        # append
        self.__comp_attr_decls.append(attr)

    def str_attr(
        self,
        name: str,
        desc: str,
        is_list: bool,
        is_optional: bool,
        default_value: Union[List[str], str] = None,
        allowed_values: List[str] = None,
        list_min_length_inclusive: int = None,
        list_max_length_inclusive: int = None,
    ):
        # sanity checks
        self._check_reserved_words(name)

        if is_optional and default_value is None:
            raise CompDeclError(
                f"attr {name}: default_value must be provided if optional."
            )

        if allowed_values is not None and default_value is not None:
            if is_list:
                for v in default_value:
                    if v not in allowed_values:
                        raise CompDeclError(
                            f"default_value {v} is not in allowed_values {allowed_values}"
                        )
            else:
                if default_value not in allowed_values:
                    raise CompDeclError(
                        f"default_value {default_value} is not in allowed_values {allowed_values}"
                    )

        if (
            list_min_length_inclusive is not None
            and list_max_length_inclusive is not None
            and list_min_length_inclusive > list_max_length_inclusive
        ):
            raise CompDeclError(
                f"list_min_length_inclusive {list_min_length_inclusive} should not be greater than list_max_length_inclusive {list_max_length_inclusive}."
            )

        # create pb
        node = AttributeDef(
            name=name,
            desc=clean_text(desc),
            type=AttrType.AT_STRINGS if is_list else AttrType.AT_STRING,
            atomic=AttributeDef.AtomicAttrDesc(
                is_optional=is_optional,
            ),
        )

        if default_value is not None:
            if is_list:
                node.atomic.default_value.ss.extend(default_value)
            else:
                node.atomic.default_value.s = default_value

        if allowed_values is not None:
            node.atomic.allowed_values.ss.extend(allowed_values)

        if is_list:
            if list_min_length_inclusive is not None:
                node.atomic.list_min_length_inclusive = list_min_length_inclusive
            else:
                node.atomic.list_min_length_inclusive = 0

            if list_max_length_inclusive is not None:
                node.atomic.list_max_length_inclusive = list_max_length_inclusive
            else:
                node.atomic.list_max_length_inclusive = -1

        # append
        self.__comp_attr_decls.append(node)

    def bool_attr(
        self,
        name: str,
        desc: str,
        is_list: bool,
        is_optional: bool,
        default_value: Union[List[bool], bool] = None,
        list_min_length_inclusive: int = None,
        list_max_length_inclusive: int = None,
    ):
        # sanity checks
        self._check_reserved_words(name)

        if is_optional and default_value is None:
            raise CompDeclError(
                f"attr {name}: default_value must be provided if optional."
            )

        if (
            list_min_length_inclusive is not None
            and list_max_length_inclusive is not None
            and list_min_length_inclusive > list_max_length_inclusive
        ):
            raise CompDeclError(
                f"list_min_length_inclusive {list_min_length_inclusive} should not be greater than list_max_length_inclusive {list_max_length_inclusive}."
            )

        # create pb
        node = AttributeDef(
            name=name,
            desc=clean_text(desc),
            type=AttrType.AT_BOOLS if is_list else AttrType.AT_BOOL,
            atomic=AttributeDef.AtomicAttrDesc(
                is_optional=is_optional,
            ),
        )

        if default_value is not None:
            if is_list:
                node.atomic.default_value.bs.extend(default_value)
            else:
                node.atomic.default_value.b = default_value

        if is_list:
            if list_min_length_inclusive is not None:
                node.atomic.list_min_length_inclusive = list_min_length_inclusive
            else:
                node.atomic.list_min_length_inclusive = 0

            if list_max_length_inclusive is not None:
                node.atomic.list_max_length_inclusive = list_max_length_inclusive
            else:
                node.atomic.list_max_length_inclusive = -1

        # append
        self.__comp_attr_decls.append(node)

    def io(
        self,
        io_type: IoType,
        name: str,
        desc: str,
        types: List[DistDataType],
        col_params: List[TableColParam] = None,
    ):
        # create pb
        self._check_reserved_words(name)
        types = [str(t) for t in types]
        io_def = IoDef(
            name=name,
            desc=clean_text(desc),
            types=types,
        )

        if col_params is not None:
            for col_param in col_params:
                col = io_def.attrs.add()
                col.name = col_param.name
                col.desc = clean_text(col_param.desc)
                if col_param.col_min_cnt_inclusive is not None:
                    col.col_min_cnt_inclusive = col_param.col_min_cnt_inclusive
                if col_param.col_max_cnt_inclusive is not None:
                    col.col_max_cnt_inclusive = col_param.col_max_cnt_inclusive

        check_io_def(io_def)

        # append
        if io_type == IoType.INPUT:
            self.__input_io_decls.append(io_def)
        else:
            self.__output_io_decls.append(io_def)

    def definition(self):
        if self.__definition is None:
            comp_def = ComponentDef(
                domain=self.domain,
                name=self.name,
                desc=self.desc,
                version=self.version,
            )

            for a in self.__comp_attr_decls:
                if a.name in self.__argnames:
                    raise CompDeclError(f"attr {a.name} is duplicate.")
                self.__argnames.add(a.name)
                new_a = comp_def.attrs.add()
                new_a.CopyFrom(a)

            for io in self.__input_io_decls:
                if io.name in self.__argnames:
                    raise CompDeclError(f"input {io.name} is duplicate.")
                self.__argnames.add(io.name)

                for input_attr in io.attrs:
                    input_attr_full_name = "_".join([io.name, input_attr.name])
                    self.__argnames.add(input_attr_full_name)

                new_io = comp_def.inputs.add()
                new_io.CopyFrom(io)

            for io in self.__output_io_decls:
                if io.name in self.__argnames:
                    raise CompDeclError(f"output {io.name} is duplicate.")
                self.__argnames.add(io.name)
                new_io = comp_def.outputs.add()
                new_io.CopyFrom(io)

            self.__definition = comp_def

        return self.__definition
