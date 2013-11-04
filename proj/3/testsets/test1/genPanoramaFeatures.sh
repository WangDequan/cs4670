# Executables
FEATURES=../Features.exe
PANORAMA=../Panorama.exe

# Parameters
FOCAL=595
K1=-0.15
K2=0.0
BLENDWIDTH=10.0
N_RANSAC=200
RANSAC_THRESH=4

# Warp images
echo "Warping image 8"
$PANORAMA sphrWarp pano1_0008.tga test1.warp.tga $FOCAL $K1 $K2

echo "Warping image 9"
$PANORAMA sphrWarp pano1_0009.tga test2.warp.tga $FOCAL $K1 $K2

# Extract features
echo "Extracting features from image 8"
$FEATURES computeFeatures test1.warp.tga test1.warp.f 2 2

echo "Extracting features from image 9"
$FEATURES computeFeatures test2.warp.tga test2.warp.f 2 2

# Match and align images
sh ./genFeaturesMatchScript.sh > match_align.txt # Generates the script
sh ./match_align.txt # Executes the script

# Now, create the panorama!
$PANORAMA blendPairs pairlist.txt panorama.tga $BLENDWIDTH
