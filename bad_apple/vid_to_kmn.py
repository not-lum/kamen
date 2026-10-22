import cv2 as cv
import sys

if len(sys.argv) != 3:
    print(f"Usage: python3 {sys.argv[0]} <vid_file> <output_kmn_file>")
    exit() 

cap = cv.VideoCapture(sys.argv[1])
WIDTH = 80
HEIGHT = 60
DELAY = 33000

def is_white(r, g, b):
    # https://stackoverflow.com/questions/6442118/python-measuring-pixel-brightness
    return (r * 0.2126 + g * 0.7152 + b * 0.0722) >= 130

with open(sys.argv[2], "w") as f:
    f.write("dispclr\n")

    old_frame = {}
    first_frame = True

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        frame = cv.resize(frame, (WIDTH, HEIGHT))

        # height, width, _ = frame.shape
        # print("height, width:", height, width)

        for x in range(0, WIDTH):
            for y in range(0, HEIGHT):
                new_pix_is_white = is_white(frame[y, x, 2], frame[y, x, 1], frame[y, x, 0])

                if not first_frame:
                    first_frame = False
                    old_pix_is_white = is_white(old_frame[y, x, 2], old_frame[y, x, 1], old_frame[y, x, 0])

                    if (new_pix_is_white == old_pix_is_white):
                        continue

                if new_pix_is_white:
                    f.write("push 1\n")
                else:
                    f.write("push 0\n")

                f.write(f"set {WIDTH * y + x}\n")

        f.write(f"disprst\ndisp\nsleep {DELAY}\n")

        old_frame = frame

    f.write("hlt\n")

cap.release()
