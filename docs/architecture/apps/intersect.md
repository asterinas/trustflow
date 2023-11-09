# 求交

提供了两个数据集求交集能力。求交目标在输入的属性中指定，如果不指定，会使用表结构中的id列进行求交。输出为两个数据集的inner join结果。

## 组件定义

组件定义说明:
1. 该求交算子组件属于preprocessing类别，组件名为psi，版本号为0.0.1。
2. 组件有两个输入，名字分别为input1和input2。两个inputs的数据类型都是独立表sf.table.individual。并且两个inputs都有一个attrs，名字为"key"，表示求交列名。
3. 组件仅有一个输出，名字为psi_output，是sf.table.individual类型。

以下定义内容对应component spec的ComponentDef结构。

```json
{
  "domain": "preprocessing",
  "name": "psi",
  "desc": "PSI between two parties.",
  "version": "0.0.1",
  "inputs": [
    {
      "name": "input1",
      "desc": "Individual table for party 1",
      "types": [
        "sf.table.individual"
      ],
      "attrs": [
        {
          "name": "key",
          "desc": "Column(s) used to join.",
          "col_min_cnt_inclusive": "1"
        }
      ]
    },
    {
      "name": "input2",
      "desc": "Individual table for party 2",
      "types": [
        "sf.table.individual"
      ],
      "attrs": [
        {
          "name": "key",
          "desc": "Column(s) used to join.",
          "col_min_cnt_inclusive": "1"
        }
      ]
    }
  ],
  "outputs": [
    {
      "name": "psi_output",
      "desc": "Output table",
      "types": [
        "sf.table.individual"
      ]
    }
  ]
}
```

## 组件输入
输入示例说明：
1. 指定使用的组件。和上文匹配，设置domain为preprocessing，name为psi，version为0.0.1。
2. 配置组件的参数。虽然psi组件本身没有定义参数，但是它的输入输出中定义了求交列是什么。因此它的attr_paths和attrs并不是空的，而是设置了求交列的名字。
3. 配置组件的输入。它对应的是component spec的proto中DistData结构。首先，配置第一个input，它的名字为input1（这个名字可以自定义，不需要和定义中的input名字相同）。它的类型是sf.table.individual。因为是一个表类型，我们还需要为它配置表的meta信息，也就是表结构。这里配置了它的id列、feature列、label列的列名和对应的数据类型。最后配置data_refs，填入该input对应的data_uuid为alice_uuid，文件路径为alice.csv.enc。前者用于请求数据密钥，后者用于下载实际文件。第二个input配置方式相同。
4. 配置组件的输出。填入该output对应的data_uuid为join_uuid，文件路径为join_table。
您可以会疑惑attr_paths、attrs和前面组件定义中的attrs是什么关系。其实就是将组件定义中的attrs的name按顺序填入到attr_paths中，并将属性的值按照对应顺序填入到attrs里面。其中attrs对应component spec的proto中Attribute结构，它可能会包含的成员为f、i64、s、b、fs、i64s、ss、bs，分别表示浮点数、整数、字符串、布尔值以及它们的数组形式。

另外，需要注意input和output是保留字，我们不能将这两个保留字直接作为attr_paths的值。原因相信您也猜到了，正是为了设置输入输出的attrs。
在设置输入输出的attrs时，我们需要在attr_paths中填入{input|output}/{IoDef name}/{TableAttrDef name}，{IoDef name}就是我们在组件定义中定义的输入输出的名字，{TableAttrDef name}就是我们在组件定义中定义的输入输出的属性。在我们这个例子中，就是填"input/input1/key"和"input/input2/key"。对应的，在attrs中填入实际值"ss": ["id"]，这就配置了本次求交的求交列为"id"，当然你也可以将它设置成"ss": ["id", "name"]，这就表示本次求交的求交列为"id"和"name"这两列。

以下输入内容对应component spec的proto中的NodeEvalParam结构。
```json
{
  "sf_node_eval_param": {
    "domain": "preprocessing",
    "name": "psi",
    "version": "0.0.1",
    "attr_paths": [
      "input/input1/key",
      "input/input2/key"
    ],
    "attrs": [
      {
        "ss": [
          "id"
        ]
      },
      {
        "ss": [
          "id"
        ]
      }
    ],
    "inputs": [
      {
        "name": "input1",
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
              "mean smoothness"
            ],
            "id_types": [
              "int"
            ],
            "feature_types": [
              "float",
              "float",
              "float",
              "float",
              "float"
            ]
          }
        },
        "data_refs": [
          {
            "uri": "file://input/?id=alice_uuid&&uri=/host/testdata/breast_cancer/alice.csv.enc"
          }
        ]
      },
      {
        "name": "input2",
        "type": "sf.table.individual",
        "meta": {
          "@type": "type.googleapis.com/opensecretflow.spec.v1.IndividualTable",
          "schema": {
            "ids": [
              "id"
            ],
            "features": [
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
              "float"
            ],
            "label_types": [
              "bool"
            ]
          }
        },
        "data_refs": [
          {
            "uri": "file://input/?id=bob_uuid&&uri=/host/testdata/breast_cancer/bob.csv.enc"
          }
        ]
      }
    ],
    "output_uris": [
      "file://output/?id=join_uuid&&uri=/host/testdata/breast_cancer/join_table"
    ]
  }
}
```
