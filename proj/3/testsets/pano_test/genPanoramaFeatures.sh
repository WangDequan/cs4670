# Executables
FEATURES=../Features.exe
PANORAMA=../Panorama.exe

# Parameters
FOCAL=595
K1=-0.15
K2=0.0
BLENDWIDTH=100.0

# Warp images
for d in *.[Jj][Pp][Gg]
do 
    echo Warping image $d
    $PANORAMA sphrWarp $d `echo $d | sed 's/.[Jj][Pp][Gg]//'`.warp.tga $FOCAL $K1 $K2
done

# Extract features
for d in *.tga
do 
    echo "Extracting features from image $d"
    $FEATURES computeFeatures $d `basename $d .tga`.f 2 2
done

# Match and align images
sh ./genFeaturesMatchScript.sh > match_align.txt # Generates the script
sh ./match_align.txt # Executes the script

# Now, create the panorama!
$PANORAMA blendPairs pairlist.txt panorama.tga $BLENDWIDTH
