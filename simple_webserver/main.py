from flask import Flask, request, jsonify
from PIL import Image
import uuid

app = Flask(__name__)

@app.route("/ping")
def index():
  return "pong"

@app.route("/upload", methods=["POST"])
def upload():
  file = request.files['image']
  # Read the image via file.stream
  img = Image.open(file.stream)
  filename = str(uuid.uuid4()) + ".jpg"
  img.save(filename)

  return jsonify({'msg': 'success', 'saved_file': filename, 'size': [img.width, img.height]})

if __name__ == "__main__":
  app.run(host='0.0.0.0', port=5000, debug=True)