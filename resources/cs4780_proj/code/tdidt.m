function tree = tdidt(data, classi, maxrec, tree)
  t1 = cputime;

  inc = 1;
  n = size(data,1);
  if n > 0
    nattrib = size(data,2);
    classes = unique(classi);
    if nargin < 4
      tree = tdidtrec(data, classi, maxrec, inc, nattrib, classes);
    else
      tree = tdidtrec(data, classi, maxrec, inc, nattrib, classes, tree, 1);
    end
  else
    tree = [];
  end

  t2 = cputime;
  disp(t2-t1);
end

function [tree, full] = tdidtrec(data, classi, maxrec, inc, nattrib, classes, tree, p)
  % attribute, threshold, left index, right index in tree
  % left index yes, right index no
  % classes are negative indexes, 0 is 0
  if nargin < 7 || p < 0 || numel(tree) == 1
    [attrib, thresh] = tdidtrow(data, classi, nattrib, classes);
    pass = logical(0);
  else
    attrib = tree(p,1);
    thresh = tree(p,2);
    pass = logical(1);
  end
  if attrib == -1
    tree = -mode(classi);
    full = 0;
  else
    treeli = (data(:, attrib) < thresh);
    treeri = ~treeli;
    treelic = classi(treeli);
    treeric = classi(treeri);
    [maxl,fmodel] = mode(treelic);
    [maxr,fmoder] = mode(treeric);
    fulll = 0;
    fullr = 0;
    inc = inc + 1;
    inch = inc;
    if (fmodel == sum(treeli)) || (maxrec < 1)
      treel = -maxl;
    else
      if pass
        [treel, fulll] = tdidtrec(data(treeli,:), treelic, maxrec - 1, inc, nattrib, classes, tree, tree(p,3));
      else
        [treel, fulll] = tdidtrec(data(treeli,:), treelic, maxrec - 1, inc, nattrib, classes);
      end
      if fulll
        inch = inch + size(treel, 1);
      end
    end

    if (fmoder == sum(treeri)) || (maxrec < 1)
      treer = -maxr;
    elseif pass
      [treer, fullr] = tdidtrec(data(treeri,:), treeric, maxrec - 1, inch, nattrib, classes, tree, tree(p,4));
    else
      [treer, fullr] = tdidtrec(data(treeri,:), treeric, maxrec - 1, inch, nattrib, classes);
    end
    full = 1;
    if fullr && fulll
      tree = [attrib, thresh, inc, inch; treel; treer];
    elseif fullr
      tree = [attrib, thresh, treel, inc; treer];
    elseif fulll
      tree = [attrib, thresh, inc, treer; treel];
    elseif treel == treer
      tree = treel;
      full = 0;
    else
      tree = [attrib, thresh, treel, treer];
    end
  end
end

function [bestattrib, bestthresh] = tdidtrow(data, classi, nattrib, classes)
  bestattrib = -1;
  bestinfo = -Inf;
  bestthresh = -1;
  for i=1:nattrib
    subd = full(data(:,i));
    threshes = unique(subd);
    tsize = size(threshes,1);
    for t=2:tsize % one not needed because empty full split
      subdl = (subd < threshes(t));
      subdr = ~subdl;
      subdrc = sum(subdr);
      subdlc = sum(subdl);
      pxl = nonzeros(histc(classi(subdl),classes));
      pxr = nonzeros(histc(classi(subdr),classes));
      pxl = pxl .* log2(pxl / subdlc);
      pxr = pxr .* log2(pxr / subdrc);
      infogain = sum(pxl) + sum(pxr);
      if infogain > bestinfo
        bestinfo = infogain;
        bestattrib = i;
        bestthresh = threshes(t);
      end
    end
  end
end
