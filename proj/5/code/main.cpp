#include "Common.h"
#include "Utils.h"
#include "Feature.h"
#include "SupportVectorMachine.h"
#include "PrecisionRecall.h"
#include "ObjectDetector.h"
#include "SubBandImagePyramid.h"
#include "FileIO.h"
#include "ImageDatabase.h"

#include "GUIController.h"
#include "GUIModel.h"

void
printUsage(const std::string &execName)
{
    printf("Usage:\n");
    printf("\t%s -h\n", execName.c_str());
    printf("\t%s TRAIN   -p <in:params> [-c <svm C param>] <in:database> <out:svm model>\n", execName.c_str());
    printf("\t%s TRAIN   -f <feature type> [-c <svm C param>] <in:database> <out:svm model>\n", execName.c_str());
    printf("\t%s PRED    <in:database> <in:svm model> [<out:prcurve.pr>] [<out:database.preds>]\n", execName.c_str());
    printf("\t%s PREDSL  [-p <in:params>] <in:database> <in:svm model> [<out:prcurve.pr>] [<out:database.preds>]\n", execName.c_str());
}

void
parseCommandLineOptions(int argc, char **argv,
                        std::vector<std::string> &args,
                        std::map<std::string, std::string> &opts)
{
    for (int i = 0; i < argc; i++) {
        if(strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            exit(EXIT_SUCCESS);
        }

        if(argv[i][0] == '-') {
            opts[argv[i]] = argv[i + 1];
            i++;
        } else {
            args.push_back(argv[i]);
        }
    }
}

int
mainSVMTrain(const std::vector<std::string> &args, const std::map<std::string, std::string> &opts)
{
    if(args.size() < 3) {
        throw CError("ERROR: Incorrect number of arguments. Run command with flag -h for help.");
    }

    std::string dbFName = args[2];
    std::string svmModelFName = args[3];

    CroppedImageDatabase db(dbFName.c_str());
    std::cout << db << std::endl;

    ParametersMap featParams;
    if(opts.count("-f") == 1) {
        std::string featureType = opts.at("-f");
        featParams = FeatureExtractor::getDefaultParameters(featureType);
    } else if(opts.count("-p") == 1) {
        std::string paramsFName = opts.at("-p");

        std::map<std::string, ParametersMap> allParams;
        loadFromFile(paramsFName, allParams);

        if(allParams.count(FEATURE_EXTRACTOR_KEY)) {
            PRINT_MSG("Using feature extractor paramaters from file ");
            featParams = allParams[FEATURE_EXTRACTOR_KEY];
        } else {
            PRINT_MSG("Using default parameters for feature extractor");
        }
    } else {
        throw CError("ERROR: Incorrect number of arguments. Run command with flag -h for help.");
    }

    double svmC = 0.01;
    if(opts.count("-c") == 1) svmC = atof(opts.at("-c").c_str());

    FeatureExtractor *featExtractor = FeatureExtractor::create(featParams);

    PRINT_MSG("Feature type: " << featExtractor->getFeatureType());

    PRINT_MSG("Loading one image to get its size");
    CByteImage img;
    ReadFile(img, db.getFilename(0).c_str());

    PRINT_MSG("Extracting features");
    FeatureCollection features;
    (*featExtractor)(db, features);

    PRINT_MSG("Training SVM");
    SupportVectorMachine svm;
    Size roiSize = db.getROISize();
    roiSize.width *= double(features[0].Shape().width) / img.Shape().width;
    roiSize.height *= double(features[0].Shape().height) / img.Shape().height;

    svm.train(db.getLabels(), features, roiSize, svmC);

    saveToFile(svmModelFName, svm, featExtractor);

    delete featExtractor;
    return EXIT_SUCCESS;
}

int
mainSVMPredict(const std::vector<std::string> &args, const std::map<std::string, std::string> &opts)
{
    if(args.size() < 3 || args.size() > 6) {
        throw CError("ERROR: Incorrect number of arguments. Run command with flag -h for help.");
    }

    std::string dbFName = args[2];
    std::string svmModelFName = args[3];
    std::string prFName = (args.size() >= 4) ? args[4] : NULL;
    std::string predsFName = (args.size() >= 5) ? args[5] : NULL;

    CroppedImageDatabase db(dbFName.c_str());
    std::cout << db << std::endl;

    PRINT_MSG("Loading SVM model and feature extractor from file");
    SupportVectorMachine svm;
    FeatureExtractor *featExtractor = NULL;
    loadFromFile(svmModelFName, svm, &featExtractor);

    PRINT_MSG("Extracting features");
    FeatureCollection features;
    (*featExtractor)(db, features);

    PRINT_MSG("Predicting");
    std::vector<float> preds = svm.predict(features);

    PRINT_MSG("Computing Precision Recall Curve");
    PrecisionRecall pr(db.getLabels(), preds);
    PRINT_MSG("Average precision: " << pr.getAveragePrecision());

    if(prFName.size() != 0) pr.save(prFName.c_str());
    if(predsFName.size() != 0) {
        CroppedImageDatabase predsDb(preds, db.getFilenames());
        predsDb.save(predsFName.c_str());
    }

    return EXIT_SUCCESS;
}

int
mainSVMPredictSlidingWindow(const std::vector<std::string> &args, const std::map<std::string, std::string> &opts)
{
    // Detection over multiple scales with non maxima suppression
    if(args.size() < 4) {
        throw CError("ERROR: Incorrect number of arguments. Run command with flag -h for help.");
    }

    std::string dbFName = args[2];
    std::string svmModelFName = args[3];
    std::string prFName = (args.size() >= 5) ? args[4] : "";
    std::string predsFName = (args.size() >= 6) ? args[5] : "";

    ParametersMap imPyrParams = SBFloatPyramid::getDefaultParameters();
    ParametersMap obDetParams = ObjectDetector::getDefaultParameters();
    if(opts.count("-p") == 1) {
        std::string paramsFName = opts.at("-p");

        std::map<std::string, ParametersMap> allParams;
        loadFromFile(paramsFName, allParams);

        if(allParams.count(IMAGE_PYRAMID_KEY)) {
            PRINT_MSG("Using image pyarmid paramaters from file ");
            imPyrParams = allParams[FEATURE_EXTRACTOR_KEY];
        } else {
            PRINT_MSG("Using default parameters for image pyaramid");
        }

        if(allParams.count(OBJECT_DETECTOR_KEY)) {
            PRINT_MSG("Using NMS paramaters from file ");
            obDetParams = allParams[FEATURE_EXTRACTOR_KEY];
        } else {
            PRINT_MSG("Using default parameters for NMS");
        }
    }

    PRINT_MSG("Loading image database");
    ImageDatabase db(dbFName);

    PRINT_MSG("Loading SVM model and features extractor from file");
    SupportVectorMachine svm;
    FeatureExtractor *featExtractor = NULL;
    loadFromFile(svmModelFName, svm, &featExtractor);

    PRINT_MSG("Initializing object detector");
    ObjectDetector obdet(obDetParams);

    std::vector<std::vector<Detection> > dets(db.getSize());
    for(int i = 0; i < db.getSize(); i++) {
        PRINT_MSG("Processing image " << std::setw(4) << (i + 1) << " of " << db.getSize());

        std::string imgFName = db.getFilenames()[i];

        // load image
        CByteImage img;
        ReadFile(img, imgFName.c_str());

        // build pyramid
        CFloatImage imgF(img.Shape());
        TypeConvert(img, imgF);
        SBFloatPyramid imgPyr(imgF, imPyrParams);

        // Extracting features on image pyramid
        FeaturePyramid featPyr;
        (*featExtractor)(imgPyr, featPyr);

        // Computing SVM response for every level
        SBFloatPyramid responsePyr;
        svm.predictSlidingWindow(featPyr, responsePyr);

        // Extracting detections from response pyramid
        obdet(responsePyr, svm.getROISize(), featExtractor->scaleFactor(), dets[i]);
    }

    ImageDatabase predsDb(dets, db.getFilenames());

    PRINT_MSG("Computing Precision Recall Curve");
    std::vector<float> labels, response;
    computeLabels(db.getDetections(), predsDb.getDetections(), labels, response);

    PRINT_MSG("Computing Precision Recall Curve");
    PrecisionRecall pr(labels, response);
    PRINT_MSG("Average precision: " << pr.getAveragePrecision());

    if(predsFName.size()) predsDb.save(predsFName.c_str());
    if(prFName.size()) pr.save(prFName.c_str());

    return EXIT_SUCCESS;
}

int
main(int argc, char **argv)
{
    fl_register_images();


    std::vector<std::string> args;
    std::map<std::string, std::string> opts;
    parseCommandLineOptions(argc, argv, args, opts);

    try {
        if(args.size() == 1) {
            GUIModel model;
            GUIController gui;

            gui.setModel(&model);
            model.setController(&gui);

            gui.show();

            return Fl::run();

        } else {
            if (strcasecmp(args[1].c_str(), "TRAIN") == 0) {
                return mainSVMTrain(args, opts);
            } else if (strcasecmp(args[1].c_str(), "PRED") == 0) {
                return mainSVMPredict(args, opts);
            } else if (strcasecmp(args[1].c_str(), "PREDSL") == 0) {
                return mainSVMPredictSlidingWindow(args, opts);
            } else {
                printUsage(args[0]);
                return EXIT_FAILURE;
            }
        }
    } catch(CError err) {
        std::cerr << "===================================================" << std::endl;
        std::cerr << "ERROR: Uncought exception:\n\t" << err.message << std::endl;
        std::cerr << "===================================================" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

