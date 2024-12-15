
from ultralytics import YOLO

import cv2

model = YOLO("best.pt")

results = model.predict(source="demo.mp4", show = True)
print(results)