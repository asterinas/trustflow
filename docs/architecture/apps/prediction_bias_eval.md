# 预测偏差评估

对预测结果进行预测偏差评估。

## 组件定义
1. 参数
    (1) bucket_num: 分桶数目。
    (2) min_item_cnt_per_bucket: 每个分桶允许包含的最小样本条数。
    (3) bucket_method: 分桶方法，取值可以是“equal_width”或者“equal_frequency”，分别对应等宽和等频。
2. 输入：预测结果。
3. 输出：输出的eport内容为分箱报告，具体包含每个分箱的avg_prediction、avg_label和bias。

```json
{
  "domain": "ml.eval",
  "name": "prediction_bias_eval",
  "desc": "Calculate prediction bias, ie. average of predictions - average of labels.",
  "version": "0.0.1",
  "attrs": [
    {
      "name": "bucket_num",
      "desc": "Num of bucket.",
      "type": "AT_INT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "i64": "10"
        },
        "lower_bound_enabled": true,
        "lower_bound": {
          "i64": "1"
        },
        "lower_bound_inclusive": true
      }
    },
    {
      "name": "min_item_cnt_per_bucket",
      "desc": "Min item cnt per bucket. If any bucket doesn't meet the requirement, error raises. For security reasons, we require this parameter to be at least 2.",
      "type": "AT_INT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "i64": "2"
        },
        "lower_bound_enabled": true,
        "lower_bound": {
          "i64": "2"
        },
        "lower_bound_inclusive": true
      }
    },
    {
      "name": "bucket_method",
      "desc": "Bucket method.",
      "type": "AT_STRING",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "s": "equal_width"
        },
        "allowed_values": {
          "ss": [
            "equal_width",
            "equal_frequency"
          ]
        }
      }
    }
  ],
  "inputs": [
    {
      "name": "predictions",
      "desc": "Input table with predictions.",
      "types": [
        "sf.table.individual"
      ],
      "attrs": [
        {
          "name": "label",
          "desc": "The real value column name",
          "col_min_cnt_inclusive": "1",
          "col_max_cnt_inclusive": "1"
        },
        {
          "name": "score",
          "desc": "The score value column name",
          "col_min_cnt_inclusive": "1",
          "col_max_cnt_inclusive": "1"
        }
      ]
    }
  ],
  "outputs": [
    {
      "name": "reports",
      "desc": "Output report.",
      "types": [
        "sf.report"
      ]
    }
  ]
}
```