# Python 3 - .bmp convert to instruction
# https://pillow.readthedocs.io/en/4.0.x/reference/Image.html
from PIL import Image

filename = "mangga.bmp"
bitmap = Image.open(filename)
output = open("binary.txt","w")

pixelarray = bitmap.load()

limit = 4096
counter = 0
for j in range(bitmap.height):
    for i in range(bitmap.width):
        if counter == limit:
            counter = 0
            output.write("\n")

        if (pixelarray[i,j] == 1) and counter < limit:
            output.write("1")
            counter += 1
        elif (pixelarray[i,j] == 0) and counter < limit:
            output.write("0")
            counter += 1
    output.write("n")
