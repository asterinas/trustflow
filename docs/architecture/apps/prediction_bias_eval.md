# 预测偏差评估

对预测结果进行预测偏差评估。

## 组件定义
1. 参数
    (1) bucket_num: 分桶数目。
    (2) min_item_cnt_per_bucket: 每个分桶允许包含的最小样本条数。
    (3) bucket_method: 分桶方法，取值可以是“equal_width”或者“equal_frequency”，分别对应等宽和等频。
2. 输入：预测结果。
3. 输出：输出的eport内容为分箱报告，具体包含每个分箱的avg_prediction、avg_label和bias。