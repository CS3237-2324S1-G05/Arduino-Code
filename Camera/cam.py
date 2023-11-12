from flask import Flask, request

app = Flask(__name__)

# Global variable to keep track of the image count
image_count = 0

@app.route('/upload', methods=['POST'])
def upload_file():
    global image_count  # Refer to the global variable
    image_data = request.data
    if image_data:
        image_count += 1  # Increment the image count
        # Use the image count to create a new filename for each image
        filename = f"{image_count}_received_image.jpg"
        with open(filename, 'wb') as f:
            f.write(image_data)
        return f"Image successfully saved as {filename}", 200
    return "Failed to save image", 400

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=3237)
