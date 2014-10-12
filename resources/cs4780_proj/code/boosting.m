function pred = boosted(traindata, trainclass, testdata, testclass)
% lets standardize on algorithms with parameters that range from 0 to 1.
% we'll put in parameters in that range. each function should return a number
% from -1 to 1 that represents both its prediction and the strength of that
% prediction.
% ideally, the functions should accept a parameter vector and process it
% without error, merely adding a dimension to the result.  This will take
% some matlab magic.  After we have predictions with strengths from each
% classifier, we can find best fit weightings using any of the algorithms
% again.

  step = .1;
  algorithms = 4;

  ratios = 0:.1:1;
  n = size(traindata, 1);
  split = ceil(n/2);
  traindata2 = traindata(split+1:end,:);
  trainclass2 = trainclass(split+1:end)
  traindata = traindata(1:split,:);
  trainclass = trainclass(1:split);
  r = size(ratios, 2);

  pred = zeros(n, alorithms * r);
  
  pred(:, 1:r) = knn(traindata, trainclass, traindata2, ratios);
  pred(:, r+1:2r) = tdidit(traindata, trainclass, traindata2, ratios);
% maybe done by bash and then imported?
  pred(:, 2r+1:3r) = svm(traindata, trainclass, traindata2, ratios);
  pred(:, 3r+1:end) = bayes(traindata, trainclass, testdata2, ratios);

% now analyze the predictoins

% we have a matrix, A, we want to multiply A times something, so we want Ax = b,
% a standard linear least squares approximation

  x = lsqnonneg(pred, testclass);
  boostedpred = pred * x;
  boostedmatches = sum(sign(boostedpred) == testclass);

  algomatches = zeros(algorithms * r,1);
  pred = sign(pred);
  parfor i=1:size(algomatches,1)
    algomatches(i) = sum(pred(:,i) == testclass);
  end

  disp algomatches;
  disp max(algomatches);
  disp boostedmatches;
  disp n;
