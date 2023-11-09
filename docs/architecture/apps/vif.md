# VIF

计算独立数据集的方差膨胀因子VIF（Variance Inflation Factor）。

## 组件定义

1. 输入：数据集，以及要计算的列。如果不选择列，则默认计算所有列。
2. 输出：VIF结果。

```json
{
  "domain": "stats",
  "name": "vif",
  "desc": "Calculate Variance Inflation Factor(VIF) for individual dataset",
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
          "desc": "Column(s) used to join. If not provided, ids of the Specify which features to calculate VIF with. If empty, all features will be used."
        }
      ]
    }
  ],
  "outputs": [
    {
      "name": "report",
      "desc": "Output Variance Inflation Factor(VIF) report.",
      "types": [
        "sf.report"
      ]
    }
  ]
}
```