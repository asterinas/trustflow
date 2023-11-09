# 全表统计

全表统计对数据进行统计，并输出统计信息。

输出信息包含每一列的以下信息。
1. datatype（数据类型）
2. total_count（总数）
3. count（非nan总数）
4. count_na（nan总数）
5. min
6. max
7. var
8. std
9. sem(standard error of the mean)
10. skewness(偏度)
11. kurtosis(峰度)
12. q1(分位数)
13. q2
14. q3
15. moment_2
16. moment_3
17. moment_4
18. central_moment_2
19. central_moment_3
20. central_moment_4
21. sum
22. sum_2
23. sum_3
24. sum_4
    ● moment_2 表示 E[X^2]
    ● central_moment_2 表示 E[（X - mean（X））^2]
    ● sum_2 表示 sum（X^2）

## 组件定义

```json
{
  "domain": "stats",
  "name": "table_statistics",
  "desc": "Get a table of statistics,\nincluding each column's\n1. datatype\n2. total_count\n3. count\n4. count_na\n5. min\n6. max\n7. var\n8. std\n9. sem\n10. skewness\n11. kurtosis\n12. q1\n13. q2\n14. q3\n15. moment_2\n16. moment_3\n17. moment_4\n18. central_moment_2\n19. central_moment_3\n20. central_moment_4\n21. sum\n22. sum_2\n23. sum_3\n24. sum_4\n- moment_2 means E[X^2].\n- central_moment_2 means E[(X - mean(X))^2].\n- sum_2 means sum(X^2).",
  "version": "0.0.1",
  "inputs": [
    {
      "name": "input_data",
      "desc": "Input table.",
      "types": [
        "sf.table.individual"
      ]
    }
  ],
  "outputs": [
    {
      "name": "report",
      "desc": "Output table statistics report.",
      "types": [
        "sf.report"
      ]
    }
  ]
}
```