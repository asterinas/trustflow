# 相关系数矩阵

计算数据集的皮尔逊乘积矩相关系数。

## 组件定义

1. 输入：数据集以及选中的特征列。如果不选择列，则会对所有列进行计算。
2. 输出:结果为一个report，report的内容为一张相关系数表。

```json
{
  "domain": "stats",
  "name": "pearsonr",
  "desc": "Calculate Pearson's product-moment correlation coefficient for individual dataset.",
  "version": "0.0.1",
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
          "desc": "Specify which features to calculate correlation coefficient with. If empty, all features will be used"
        }
      ]
    }
  ],
  "outputs": [
    {
      "name": "report",
      "desc": "Output Pearson's product-moment correlation coefficient report.",
      "types": [
        "sf.report"
      ]
    }
  ]
}
```

## 结果示例

输出内容的json形式如下。
```json
{
  "name": "corr",
  "desc": "corr table",
  "tabs": [
    {
      "divs": [
        {
          "children": [
            {
              "type": "table",
              "table": {
                "headers": [
                  {
                    "name": "SALARY",
                    "type": "float"
                  },
                  {
                    "name": "AGE",
                    "type": "float"
                  }
                ],
                "rows": [
                  {
                    "name": "SALARY",
                    "items": [
                      {
                        "f": 1.0
                      },
                      {
                        "f": 0.8728716
                      }
                    ]
                  },
                  {
                    "name": "AGE",
                    "items": [
                      {
                        "f": 0.8728716
                      },
                      {
                        "f": 1.0
                      }
                    ]
                  }
                ]
              }
            }
          ]
        }
      ]
    }
  ]
}
```

用表格来展示如下。

|        | SALARY     | AGE       |
| ------ | ---------- | --------- |
| SALARY | 1          | 0.8728716 |
| AGE    | 0.8728716  | 1         |
