# WOE分箱

WOE（Weight of Evidence）用于衡量某个变量对目标变量的影响程度，WOE值越大则通常表示该分组下的数据对目标变量的影响越大，WOE在金融行业评分卡场景使用广泛。
本组件对执行的数据列进行WOE分箱，并产出分箱规则。


## 组件定义
1. WOE算子属于feature类别，组件名为vert_woe_binning，版本号为0.0.1。
2. 组件的属性包含以下。
    (1) binning_method：分箱方法，可选值为"quantile"或"bucket"，分别对应分位数和分桶；类型是AT_STRING，表示值为字符串；is_optional等于true，说明它是一个可选值，如果不填这个参数，会使用它的默认值"quantile"。
    (2)positive_label：可选值，类型为字符串，默认值为"1"，表示1表示正样本。
    (3)bin_num：可选值，默认值为10。最小值为0，表示分桶的数目。
3. 组件输入名字为input_data，类型为sf.table.individual，同时输入的属性feature_selects表示选取哪些列进行woe分箱。col_min_cnt_inclusive为1表示选取的列至少为1列。输入的属性label表示哪一列是标签列，标签列必须为1列。
4. 组件输出名字为woe_rule，类型是sf.rule.woe_binning。

以下定义内容对应component spec的ComponentDef结构。
```json
{
  "domain": "feature",
  "name": "vert_woe_binning",
  "desc": "Generate Weight of Evidence (WOE) binning rules for individual datasets.",
  "version": "0.0.1",
  "attrs": [
    {
      "name": "binning_method",
      "desc": "How to bin features with numeric types: quantile\"(equal frequency)/\"bucket\"(equal width)",
      "type": "AT_STRING",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "s": "quantile"
        },
        "allowed_values": {
          "ss": [
            "quantile",
            "bucket"
          ]
        }
      }
    },
    {
      "name": "positive_label",
      "desc": "Which value represent positive value in label.",
      "type": "AT_STRING",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "s": "1"
        }
      }
    },
    {
      "name": "bin_num",
      "desc": "Max bin counts for one features.",
      "type": "AT_INT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "i64": "10"
        },
        "lower_bound_enabled": true,
        "lower_bound": {}
      }
    }
  ],
  "inputs": [
    {
      "name": "input_data",
      "desc": "Input table.",
      "types": [
        "sf.table.individual"
      ],
      "attrs": [
        {
          "name": "feature_selects",
          "desc": "which features should be binned.",
          "col_min_cnt_inclusive": "1"
        },
        {
          "name": "label",
          "desc": "Label column.",
          "col_min_cnt_inclusive": "1",
          "col_max_cnt_inclusive": "1"
        }
      ]
    }
  ],
  "outputs": [
    {
      "name": "woe_rule",
      "desc": "Output WOE rule.",
      "types": [
        "sf.rule.woe_binning"
      ]
    }
  ]
}
```

## 组件输入
下面为输入的示例，该示例选取了"mean radius"和"mean texture"两列进行WOE分箱，binning_method为quantile，positive_label为1，bin_num为10。

以下输入内容对应component spec的NodeEvalParam结构。
```json
{
  "sf_node_eval_param": {
    "domain": "feature",
    "name": "vert_woe_binning",
    "version": "0.0.1",
    "attr_paths": [
      "binning_method",
      "positive_label",
      "bin_num",
      "input/input_data/feature_selects"
    ],
    "attrs": [
      {
        "s": "quantile"
      },
      {
        "s": "1"
      },
      {
        "i64": "10"
      },
      {
        "ss": [
          "mean radius",
          "mean texture"
        ]
      }
    ],
    "inputs": [
      {
        "name": "input_data",
        "type": "sf.table.individual",
        "meta": {
          "@type": "type.googleapis.com/opensecretflow.spec.v1.IndividualTable",
          "schema": {
            "ids": [
              "id"
            ],
            "features": [
              "mean radius",
              "mean texture",
              "mean perimeter",
              "mean area",
              "mean smoothness",
              "mean compactness",
              "mean concavity",
              "mean concave points",
              "mean symmetry",
              "mean fractal dimension"
            ],
            "labels": [
              "target"
            ],
            "id_types": [
              "int"
            ],
            "feature_types": [
              "float",
              "float",
              "float",
              "float",
              "float",
              "float",
              "float",
              "float",
              "float",
              "float"
            ],
            "label_types":[
              "bool"
            ]
          }
        },
        "data_refs": [
          {
            "uri": "file://input/?id=join_uuid&&uri=/host/testdata/breast_cancer/join_table"
          }
        ]
      }
    ],
    "output_uris": [
      "file://output/?id=woe_rule_uuid&&uri=/host/testdata/breast_cancer/woe_rule"
    ]
  }
}
```