from ultralytics import YOLO
import torch
import cv2

class CustomPredictor:
    def __init__(self, model_path):
        self.model = YOLO(model_path)

    def preprocess(self, img):
        img = torch.from_numpy(img).to(self.model.device)
        img = img.half() if self.model.fp16 else img.float()  
        img /= 255 
        return img

    def postprocess(self, preds, img, orig_img):
        preds = ops.non_max_suppression(preds, conf_thres=0.25, iou_thres=0.45)
        for i, pred in enumerate(preds):
            shape = orig_img[i].shape if isinstance(orig_img, list) else orig_img.shape
            pred[:, :4] = ops.scale_boxes(img.shape[2:], pred[:, :4], shape).round()
        return preds

    def annotate(self, img, preds):
        annotator = Annotator(img, line_width=2, example=str(self.model.names))
        for *xyxy, conf, cls in preds:
            label = f"{self.model.names[int(cls)]} {conf:.2f}"
            annotator.box_label(xyxy, label, color=colors(cls, True))
        return annotator.result()

    def predict(self, source, show=False):
        results = self.model.predict(source=source, show=show)
        return results


def main():
    model_path = "location of best.pt"

    video_source = "location of demo.mp4"

    try:
        predictor = CustomPredictor(model_path)

        results = predictor.predict(source=video_source, show=True)
        
        print(results)

    except FileNotFoundError as e:
        print(f"Error: {e}. Check if 'best.pt' and 'demo.mp4' exist in the directory.")
    except Exception as e:
        print(f"Unexpected error: {e}")


if __name__ == "__main__":
    main()
