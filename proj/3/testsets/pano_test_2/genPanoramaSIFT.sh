# Executables
FEATURES=../Features.exe
PANORAMA=../Panorama.exe
SIFT=../siftWin32.exe

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
    echo "Extracting SIFT features from image $d"

    # Silly intermediate conversion to ppm because sift refuses to work on
    # on pgms created directly from tga files
    mogrify -format ppm $d
    mogrify -format pgm `basename $d .tga`.ppm
    $SIFT < `basename $d .tga`.pgm > `basename $d .tga`.key
done

# Match and align images
sh ./genSIFTMatchScript.sh > match_align.txt # Generates the script
sh ./match_align.txt # Executes the script

# Now, create the panorama!
$PANORAMA blendPairs pairlist.txt panorama.tga $BLENDWIDTH
