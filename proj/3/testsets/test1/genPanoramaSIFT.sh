# Executables
FEATURES=../Features.exe
PANORAMA=../Panorama.exe
SIFT=../siftWin32.exe

# Parameters
FOCAL=595
K1=-0.15
K2=0.0
BLENDWIDTH=100.0
N_RANSAC=200
RANSAC_THRESH=4

# Warp images
echo Warping image 8
$PANORAMA sphrWarp pano1_0008.tga test1.warp.tga $FOCAL $K1 $K2

echo Warping image 9
$PANORAMA sphrWarp pano1_0009.tga test2.warp.tga $FOCAL $K1 $K2

# Extract features
echo "Extracting SIFT features from image 8"

# Silly intermediate conversion to ppm because sift refuses to work on
# on pgms created directly from tga files
mogrify -format ppm test1.warp.tga
mogrify -format pgm test1.warp.ppm
$SIFT < test1.warp.pgm > test1.warp.key

echo "Extracting SIFT features from image 9"

# Silly intermediate conversion to ppm because sift refuses to work on
# on pgms created directly from tga files
mogrify -format ppm test2.warp.tga
mogrify -format pgm test2.warp.ppm
$SIFT < test2.warp.pgm > test2.warp.key

# Match and align images
sh ./genSIFTMatchScript.sh > match_align.txt # Generates the script
sh ./match_align.txt # Executes the script

# Now, create the panorama!
$PANORAMA blendPairs pairlist.txt panorama.tga $BLENDWIDTH
