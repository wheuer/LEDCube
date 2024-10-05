from PIL import Image, ImageDraw

# Replicated C implementation should give the desired image ouput as a text file of hex values
# Each pixel is represented in 5-6-5 color, so 2 bytes/pixel
def convertArrayToImage():
    with open("./image.txt", "r") as file:
        # Read in image data from C output file
        values = [int(i.replace('\n', '').replace('\r', '').replace(' ', '').replace('0x', ''), 16) for i in file.read().split(',')]
        
        # Since we are going to use 8-bit grayscale whereas our input is true/false, need to scale to 0xFF or 0x00
        for pixel in range(len(values)):
            if values[pixel]:
                values[pixel] = 0xFF

        # Show and save resulting image
        image = Image.frombytes(mode="L", size=(16, 16), data=bytearray(values))
        image.show()
        image.save('./image.png')

def createTestImage():
    with open("./image.txt", "w") as file:
        for i in range((16 * 16) - 1):
            file.write(f"1,")
        file.write(f"0")

if __name__ == "__main__":
    # createTestImage()
    convertArrayToImage()

