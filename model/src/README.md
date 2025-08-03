# Model and Asset preparation

## files preparations

- **HAR model***: normal model that use to predict movement
- **rollback file***: Use to turn whats model predicts back to word


## Requirements

- Python 3.7+

## Setup & Installation

1.  **Clone the Repository**
    ```bash
    git clone <your-repository-url>
    cd <your-repository-name>
    ```

2.  **Create a Virtual Environment (Recommended)**
    ```bash
    python -m venv venv
    # On Windows
    .\venv\Scripts\activate
    # On macOS/Linux
    source venv/bin/activate
    ```

3.  **Install Dependencies**
    The required libraries are listed in `requirements.txt`. Install them using pip:
    ```bash
    cd model
    cd src
    pip install -r requirements.txt
    ```

4.  **Change file name**
    Now we need to change the model and other path
    ```bash
    with open(r"xxxxxxxxxxxxxxxxx.json",'r',encoding="utf-8") as f:
        rollback = json.load(f)
    model_path = r"xxxxxxxxxxxxxxxxx.pt"
    test_df = pd.read_csv(rf"xxxxxxxxxxxxxxxxxxxxxx.csv")
    ```
## What you need to know
in real word situation you need to replace this chunk of code
  ```bash
  test_df = pd.read_csv(rf"F:\Hybridmodel-project\Sign_Language_Detection\collect_data\20250715_111750_DATA_INDICATOR_sensor.csv")
  test_df = test_df[~(test_df.Label.isin(["error_redo","break_time"]))].reset_index(drop=True)
  test = test_df.drop(columns=["Label","timestamp_ms"]).values
  Label = test_df["Label"].values
  data = []
  pv_label = ""
  y_true = []
  y_pred = []
  for test_data,lab in zip(test,Label):
      # print(lab)
      if lab!=pv_label and pv_label!="":
          # print(f"class from {pv_label} --> {lab}")
          if len(data) < 30:
              data = []
              continue
          data = torch.tensor(data)
          print("convert_data")
          tas = convert_data(data)
          # print(tas)
          answer = torch.argmax(model(tas.unsqueeze(0)))
          finalans = rollback[str(answer.item())]
          las = labels[pv_label]
          # print(las)
          print(f"the current class is {finalans} of class {pv_label}")
          data = []
          y_true.append(las)
          y_pred.append(answer.item())
      else:
          data.append(test_data)
          
      pv_label = lab
  ```
  with a streaming method that give a data to convert data function in [[seq_lenght,28]] in a tensor.double datatype
  then model will predict the probability of all class and we will chose the heighest prob using torch.argmax and then convert it back using rollback variable
