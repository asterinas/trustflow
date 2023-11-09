# 随机分割

将数据集拆分为训练集和测试集。

## 组件定义

1. 参数
    (1) train_size: 训练集占的比例，取值范围为(0, 1)，默认值为0.75。
    (2) fix_random: 是否使用固定的随机数种子。
    (3) random_state: 随机数种子，取值为integer，默认值为1024。
    (4) shuffle: 拆分前是否对数据进行打乱，true表示打乱。
2. 输入：待拆分的数据。
3. 输出：拆分后得到的训练集和测试集。

```json
{
  "domain": "preprocessing",
  "name": "train_test_split",
  "desc": "Split datasets into random train and test subsets.\n- Please check: https://scikit-learn.org/stable/modules/generated/sklearn.model_selection.train_test_split.html",
  "version": "0.0.1",
  "attrs": [
    {
      "name": "train_size",
      "desc": "Proportion of the dataset to include in the train subset.",
      "type": "AT_FLOAT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "f": 0.75
        },
        "lower_bound_enabled": true,
        "lower_bound": {},
        "lower_bound_inclusive": true,
        "upper_bound_enabled": true,
        "upper_bound": {
          "f": 1
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "fix_random",
      "desc": "Whether to fix random.",
      "type": "AT_BOOL",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "b": true
        }
      }
    },
    {
      "name": "random_state",
      "desc": "Specify the random seed of the shuffling.",
      "type": "AT_INT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "i64": "1024"
        },
        "lower_bound_enabled": true,
        "lower_bound": {}
      }
    },
    {
      "name": "shuffle",
      "desc": "Whether to shuffle the data before splitting.",
      "type": "AT_BOOL",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "b": true
        }
      }
    }
  ],
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
      "name": "train",
      "desc": "Output train dataset.",
      "types": [
        "sf.table.individual"
      ]
    },
    {
      "name": "test",
      "desc": "Output test dataset.",
      "types": [
        "sf.table.individual"
      ]
    }
  ]
}
```