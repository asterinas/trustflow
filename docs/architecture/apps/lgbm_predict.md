# LightGBM预测

使用给定的LightGBM模型对数据进行预测。

## 组件定义

1. 参数
    (1) pred_name: 预测值的列名。
    (2) save_label: 输出结果是否包含标签列，true表示保存。
    (3) label_name: 标签列的名称，默认为“label”。
    (4) save_id: 输出结果是否保存ID列，true表示保存。
    (5) id_name： ID列的名称。
    (6) col_names: 可选，输出指定的列到结果中，默认为空。
2. 输入：待预测的数据以及LightGBM模型。
3. 输出：预测结果。

```json
{
    "domain": "ml.predict",
    "name": "lgbm_predict",
    "desc": "Predict using the lgbm model.",
    "version": "0.0.1",
    "attrs": [
        {
            "name": "pred_name",
            "desc": "Column name for predictions.",
            "type": "AT_STRING",
            "atomic": {
                "is_optional": true,
                "default_value": {
                    "s": "pred"
                }
            }
        },
        {
            "name": "save_label",
            "desc": "Whether or not to save real label column into output pred table. If true, input feature_dataset must contain label column.",
            "type": "AT_BOOL",
            "atomic": {
                "is_optional": true,
                "default_value": {}
            }
        },
        {
            "name": "label_name",
            "desc": "Column name for label.",
            "type": "AT_STRING",
            "atomic": {
                "is_optional": true,
                "default_value": {
                    "s": "label"
                }
            }
        },
        {
            "name": "save_id",
            "desc": "Whether to save id column into output pred table. If true, input feature_dataset must contain id column.",
            "type": "AT_BOOL",
            "atomic": {
                "is_optional": true,
                "default_value": {}
            }
        },
        {
            "name": "id_name",
            "desc": "Column name for id.",
            "type": "AT_STRING",
            "atomic": {
                "is_optional": true,
                "default_value": {
                    "s": "id"
                }
            }
        },
        {
            "name": "col_names",
            "desc": "Extra column names into output pred table.",
            "type": "AT_STRINGS",
            "atomic": {
                "list_max_length_inclusive": "-1",
                "is_optional": true
            }
        }
    ],
    "inputs": [
        {
            "name": "feature_dataset",
            "desc": "Input feature dataset.",
            "types": [
                "sf.table.individual"
            ],
            "attrs": [
                {
                    "name": "ids",
                    "desc": "Id columns.",
                    "col_max_cnt_inclusive": "1"
                },
                {
                    "name": "label",
                    "desc": "Label column.",
                    "col_max_cnt_inclusive": "1"
                }
            ]
        },
        {
            "name": "model",
            "desc": "Input model.",
            "types": [
                "sf.model.lgbm"
            ]
        }
    ],
    "outputs": [
        {
            "name": "pred",
            "desc": "Output prediction.",
            "types": [
                "sf.table.individual"
            ]
        }
    ]
}
```