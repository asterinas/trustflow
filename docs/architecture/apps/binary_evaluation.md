# 二分类评估

对预测结果进行二分类评估。

## 组件定义

1. 参数
    (1) bucket_num: 分桶数目。
    (2) min_item_cnt_per_bucket: 每个分桶允许包含的最小样本条数。
2. 输入：预测结果（比如LR预测的输出）。
3. 输出：结果为一个report，包含以下内容：
    (1) summary_report: 总结报告，包含total_samples、positive_samples、negative_samples、auc、ks和f1_score。
    (2) eq_frequent_bin_report: 等频分箱报告。
    (3) eq_range_bin_report: 等距分箱报告。
    (4) head_report: FPR=[0.001，0.005，0.01，0.05，0.1，0.2]的精度报告，包含fpr、precision、recall和threshold。

```json
{
  "domain": "ml.eval",
  "name": "biclassification_eval",
  "desc": "Statistics evaluation for a bi-classification model on a dataset.\n1. summary_report: SummaryReport\n2. eq_frequent_bin_report: List[EqBinReport]\n3. eq_range_bin_report: List[EqBinReport]\n4. head_report: List[PrReport]\nreports for fpr = 0.001, 0.005, 0.01, 0.05, 0.1, 0.2",
  "version": "0.0.1",
  "attrs": [
    {
      "name": "bucket_num",
      "desc": "Number of buckets.",
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
    }
  ],
  "inputs": [
    {
      "name": "predictions",
      "desc": "Input table with predictions",
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