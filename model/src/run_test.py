from pyexpat import model
import torch 
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from tqdm.notebook import tqdm
import json
from model import CNNTimeSeriesClassifier,ImprovedCustomDataset

def laodAndPreData(csv_path):
    df = pd.read_csv(rf"{csv_path}")
    x = df[~(df.Label.isin(["error_redo","break_time"]))].reset_index(drop=True)
    x = x.drop(columns=["Label","timestamp_ms"]).values
    y = x["Label"].values

    return x,y

def loadModelAndLabel(model_path,labels_path,rollback_path):
    with open(rf"{rollback_path}",'r',encoding="utf-8") as f:
        rollback = json.load(f)
    with open(rf"{labels_path}",'r',encoding="utf-8") as f:
        labels = json.load(f)

    if torch.cuda.is_available():
        model = torch.load(rf"{model_path}",weights_only=False)
        model.to("cuda")
    else:
        model = torch.load(rf"{model_path}",weights_only=False,map_location=torch.device('cpu'))

    return model,labels,rollback
    
def convert_data(features):
    chunk_size = 50
    # features = torch.rand(30,28)
    # sequences = []
    print(chunk_size,features.shape)
    
    if len(features) >= 50:
        # If longer than chunk_size, use uniform sampling
        indices = np.linspace(0, len(features)-1, chunk_size, dtype=int)
        sequence = features[indices]
    else:
        # If shorter, pad with zeros at the end
        sequence = np.zeros((chunk_size, features.shape[1]))
        sequence[:len(features)] = features

    
    if torch.cuda.is_available():
        return torch.tensor(sequence).to("cuda")
    else:
        return torch.tensor(sequence)
    

model,labels,rollback = loadModelAndLabel(model_path = None, labels_path = None, rollback_path = None)

model.double()
model.eval()


x,y = laodAndPreData(csv_path=None)
data = []
pv_label = ""
y_true = []
y_pred = []
for test_data,lab in zip(x,y):
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
    
    
from sklearn.metrics import f1_score,recall_score,accuracy_score,confusion_matrix,ConfusionMatrixDisplay
f1_scores = f1_score(y_true, y_pred, average="micro")
print("f1 score    ",f1_scores)
recall_scores = recall_score(y_true, y_pred, average="micro")
print("recal score ",recall_scores)
acc = accuracy_score(y_true, y_pred)
print("acc score   ",acc)
