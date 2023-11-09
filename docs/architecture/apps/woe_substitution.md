# WOE转换

该组件通常和WOE分箱组件配合使用，根据WOE分箱组件产出的分享规则，对指定的数据列进行分箱。

## 组件定义
1. 输入为待分箱的数据和分箱规则
2. 输出为分箱后的数据

```json
{
  "domain": "feature",
  "name": "vert_woe_substitution",
  "desc": "Substitute datasets' value by WOE substitution rules.",
  "version": "0.0.1",
  "inputs": [
    {
      "name": "input_data",
      "desc": "Dataset to be substituted.",
      "types": [
        "sf.table.individual"
      ]
    },
    {
      "name": "woe_rule",
      "desc": "WOE substitution rule.",
      "types": [
        "sf.rule.woe_binning"
      ]
    }
  ],
  "outputs": [
    {
      "name": "output_data",
      "desc": "Output substituted dataset.",
      "types": [
        "sf.table.individual"
      ]
    }
  ]
}
```