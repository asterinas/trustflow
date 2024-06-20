# LightGBM训练

使用LightGBM对数据集进行训练，得到LightGBM模型，支持二分类和线性回归。

## 组件定义

```json
{
  "domain": "ml.train",
  "name": "lgbm_train",
  "desc": "LightGBM train component for individual dataset.",
  "version": "0.0.1",
  "attrs": [
      {
          "name": "n_estimators",
          "desc": "Number of boosted trees to fit.",
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
          "name": "objective",
          "desc": "Specify the learning objective.",
          "type": "AT_STRING",
          "atomic": {
              "is_optional": true,
              "default_value": {
                  "s": "binary"
              },
              "allowed_values": {
                  "ss": [
                      "regression",
                      "binary"
                  ]
              }
          }
      },
      {
          "name": "boosting_type",
          "desc": "Boosting type.",
          "type": "AT_STRING",
          "atomic": {
              "is_optional": true,
              "default_value": {
                  "s": "gbdt"
              },
              "allowed_values": {
                  "ss": [
                      "gbdt",
                      "rf",
                      "dart"
                  ]
              }
          }
      },
      {
          "name": "learning_rate",
          "desc": "Learning rate.",
          "type": "AT_FLOAT",
          "atomic": {
              "is_optional": true,
              "default_value": {
                  "f": 0.1
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
          "name": "num_leaves",
          "desc": "Max number of leaves in one tree.",
          "type": "AT_INT",
          "atomic": {
              "is_optional": true,
              "default_value": {
                  "i64": "31"
              },
              "lower_bound_enabled": true,
              "lower_bound": {
                  "i64": "2"
              },
              "lower_bound_inclusive": true,
              "upper_bound_enabled": true,
              "upper_bound": {
                  "i64": "1024"
              },
              "upper_bound_inclusive": true
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
              "sf.model.lgbm"
          ]
      }
  ]
}
```