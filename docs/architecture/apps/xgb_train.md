# XGBoost训练

使用XGBoost对数据集进行训练，得到XGBoost模型。

## 组件定义

```json
{
  "domain": "ml.train",
  "name": "xgb_train",
  "desc": "Provides both classification and regression tree boosting (also known as GBDT, GBM) for individual dataset.",
  "version": "0.0.1",
  "attrs": [
    {
      "name": "num_boost_round",
      "desc": "Number of boosting iterations.",
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
        "lower_bound_inclusive": true,
        "upper_bound_enabled": true,
        "upper_bound": {
          "i64": "1024"
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "max_depth",
      "desc": "Maximum depth of a tree.",
      "type": "AT_INT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "i64": "6"
        },
        "lower_bound_enabled": true,
        "lower_bound": {
          "i64": "1"
        },
        "lower_bound_inclusive": true,
        "upper_bound_enabled": true,
        "upper_bound": {
          "i64": "16"
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "max_leaves",
      "desc": "Maximum leaf of a tree. 0 indicates no limit.",
      "type": "AT_INT",
      "atomic": {
        "is_optional": true,
        "default_value": {},
        "lower_bound_enabled": true,
        "lower_bound": {},
        "lower_bound_inclusive": true,
        "upper_bound_enabled": true,
        "upper_bound": {
          "i64": "32768"
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "seed",
      "desc": "Pseudorandom number generator seed.",
      "type": "AT_INT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "i64": "42"
        },
        "lower_bound_enabled": true,
        "lower_bound": {},
        "lower_bound_inclusive": true
      }
    },
    {
      "name": "learning_rate",
      "desc": "Step size shrinkage used in update to prevent overfitting.",
      "type": "AT_FLOAT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "f": 0.3
        },
        "lower_bound_enabled": true,
        "lower_bound": {},
        "upper_bound_enabled": true,
        "upper_bound": {
          "f": 1
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "lambda",
      "desc": "L2 regularization term on weights.",
      "type": "AT_FLOAT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "f": 1
        },
        "lower_bound_enabled": true,
        "lower_bound": {},
        "lower_bound_inclusive": true,
        "upper_bound_enabled": true,
        "upper_bound": {
          "f": 10000
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "gamma",
      "desc": "Greater than 0 means pre-pruning enabled. If gain of a node is less than this value, it would be pruned.",
      "type": "AT_FLOAT",
      "atomic": {
        "is_optional": true,
        "default_value": {},
        "lower_bound_enabled": true,
        "lower_bound": {},
        "lower_bound_inclusive": true,
        "upper_bound_enabled": true,
        "upper_bound": {
          "f": 10000
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "colsample_bytree",
      "desc": "Subsample ratio of columns when constructing each tree.",
      "type": "AT_FLOAT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "f": 1
        },
        "lower_bound_enabled": true,
        "lower_bound": {},
        "upper_bound_enabled": true,
        "upper_bound": {
          "f": 1
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "base_score",
      "desc": "The initial prediction score of all instances, global bias.",
      "type": "AT_FLOAT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "f": 0.5
        },
        "lower_bound_enabled": true,
        "lower_bound": {},
        "upper_bound_enabled": true,
        "upper_bound": {
          "f": 1
        }
      }
    },
    {
      "name": "min_child_weight",
      "desc": "Minimum sum of instance weight (hessian) needed in a child. If the tree partition step results in a leaf node with the sum of instance weight less than min_child_weight, then the building process will give up further partitioning",
      "type": "AT_FLOAT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "f": 1
        },
        "lower_bound_enabled": true,
        "lower_bound": {},
        "lower_bound_inclusive": true,
        "upper_bound_enabled": true,
        "upper_bound": {
          "f": 1000
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "objective",
      "desc": "Specify the learning objective.",
      "type": "AT_STRING",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "s": "binary:logistic"
        },
        "allowed_values": {
          "ss": [
            "reg:squarederror",
            "binary:logistic"
          ]
        }
      }
    },
    {
      "name": "alpha",
      "desc": "L1 regularization term on weights. Increasing this value will make model more conservative",
      "type": "AT_FLOAT",
      "atomic": {
        "is_optional": true,
        "default_value": {},
        "lower_bound_enabled": true,
        "lower_bound": {},
        "lower_bound_inclusive": true,
        "upper_bound_enabled": true,
        "upper_bound": {
          "f": 10000
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "subsample",
      "desc": "Subsample ratio of the training instance.",
      "type": "AT_FLOAT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "f": 1
        },
        "lower_bound_enabled": true,
        "lower_bound": {},
        "upper_bound_enabled": true,
        "upper_bound": {
          "f": 1
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "max_bin",
      "desc": "Maximum number of discrete bins to bucket continuous features.  Only used if tree_method is set to hist, approx or gpu_hist.",
      "type": "AT_INT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "i64": "10"
        },
        "lower_bound_enabled": true,
        "lower_bound": {},
        "upper_bound_enabled": true,
        "upper_bound": {
          "i64": "254"
        }
      }
    },
    {
      "name": "tree_method",
      "desc": "The tree construction algorithm used in XGBoost.",
      "type": "AT_STRING",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "s": "auto"
        },
        "allowed_values": {
          "ss": [
            "auto",
            "exact",
            "approx",
            "hist"
          ]
        }
      }
    },
    {
      "name": "booster",
      "desc": "Which booster to use",
      "type": "AT_STRING",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "s": "gbtree"
        },
        "allowed_values": {
          "ss": [
            "gbtree",
            "gblinear",
            "dart"
          ]
        }
      }
    }
  ],
  "inputs": [
    {
      "name": "train_dataset",
      "desc": "Input table.",
      "types": [
        "sf.table.individual"
      ],
      "attrs": [
        {
          "name": "ids",
          "desc": "Id columns will not be trained."
        },
        {
          "name": "label",
          "desc": "Label column.",
          "col_min_cnt_inclusive": "1",
          "col_max_cnt_inclusive": "1"
        }
      ]
    }
  ],
  "outputs": [
    {
      "name": "output_model",
      "desc": "Output model.",
      "types": [
        "sf.model.xgb"
      ]
    }
  ]
}
```