# LR训练

使用线性回归/逻辑回归模型对数据集进行训练，得到线性/回归模型。

## 组定定义

```json
{
  "domain": "ml.train",
  "name": "lr_train",
  "desc": "linear or logistic regression training.",
  "version": "0.0.1",
  "attrs": [
    {
      "name": "max_iter",
      "desc": "Maximum number of iterations taken for the solvers to converge.",
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
          "i64": "10000"
        },
        "upper_bound_inclusive": true
      }
    },
    {
      "name": "reg_type",
      "desc": "Regression type",
      "type": "AT_STRING",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "s": "logistic"
        },
        "allowed_values": {
          "ss": [
            "linear",
            "logistic"
          ]
        }
      }
    },
    {
      "name": "l2_norm",
      "desc": "L2 regularization term.",
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
        }
      }
    },
    {
      "name": "tol",
      "desc": "Tolerance for stopping criteria.",
      "type": "AT_FLOAT",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "f": 0.0001
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
      "name": "penalty",
      "desc": "The penalty(aka regularization term) to be used.",
      "type": "AT_STRING",
      "atomic": {
        "is_optional": true,
        "default_value": {
          "s": "l2"
        },
        "allowed_values": {
          "ss": [
            "l1",
            "l2",
            "elasticnet",
            "None"
          ]
        }
      }
    }
  ],
  "inputs": [
    {
      "name": "train_dataset",
      "desc": "Input train dataset.",
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
        "sf.model.lr"
      ]
    }
  ]
}
```