from PIL import Image

im = Image.open('image.png', 'r')
pix_val = list(im.getdata())
pix_val_flat = [x for sets in pix_val for x in sets]

# print(pix_val)
file = open('image.txt','w') 
for (r, g, b, a) in pix_val:
	file.write(str(r))
	file.write(" ")
	file.write(str(g))
	file.write(" ")
	file.write(str(b))
	file.write("\n")