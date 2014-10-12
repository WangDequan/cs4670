function pred = bayes(traindata, trainclass, testdata, testclass, cost, classes)
  if nargin < 6
    classes = [1;-1];
  end
  if nargin < 5
    cost = 1;
  end
  if nargin < 3
    testdata = traindata;
    testclass = trainclass;
  end

  nrowstrain = size(traindata,1);
  nrowstest = size(testdata,1);
  nclasses = size(classes, 1);

  if nrowstrain ~= size(trainclass,1)
    disp('illegal data - dimensions');
  elseif nrowstest ~= size(testclass,1)
    disp('illegal data - dimensions');
  end

  ncolstrain = size(traindata, 2);
  ncolstest = size(testdata,2);
  ncols = max(ncolstrain, ncolstest);

  if ncolstest < ncols
    [u,v,w] = find(testdata);
    testdata = sparse(u,v,w,nrowstest,ncols);
  elseif ncolstrain < ncols
    [u,v,w] = find(traindata);
    traindata = sparse(u,v,w,nrowstrain,ncols);
  end

  prob = zeros(nrowstest, nclasses);
  for j=1:nclasses
    cond = (trainclass == classes(j));
    trainwordcount = sum(traindata(cond,:),1)';
    pr = log(1 + trainwordcount) - log(ncols + sum(trainwordcount));
    prob(:,j) = log(sum(cond)) + testdata * pr;
  end
  prob(:,1) = prob(:,1) + log(cost);
  prob(:,2) = prob(:,2) + log(1);
  [~,pred] = max(prob, [], 2);
  pred = classes(pred);
  pred((prob(:,1) == prob(:,2))) = mode(trainclass);
end
