import imageio
import os
images = []
os.chdir("images")
print(os.listdir())

for filename in os.listdir():
    if ".bmp" in filename:
        images.append(imageio.imread(filename))
os.chdir("..")
imageio.mimsave('movie.gif', images, duration=0.05)
