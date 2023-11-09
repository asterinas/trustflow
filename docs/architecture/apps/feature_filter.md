# 特征过滤

该组件用于从数据集中删除指定的特征。

## 组件定义

1. 输入：数据集以及要删除的特征列。
2. 输出：删除指定特征列之后的数据。

```json
{
  "domain": "preprocessing",
  "name": "feature_filter",
  "desc": "Drop features from the dataset.",
  "version": "0.0.1",
  "inputs": [
    {
      "name": "in_ds",
      "desc": "Input table.",
      "types": [
        "sf.table.individual"
      ],
      "attrs": [
        {
          "name": "drop_features",
          "desc": "Features to drop.",
          "col_min_cnt_inclusive": "1"
        }
      ]
    }
  ],
  "outputs": [
    {
      "name": "out_ds",
      "desc": "Output table.",
      "types": [
        "sf.table.individual"
      ]
    }
  ]
}
```