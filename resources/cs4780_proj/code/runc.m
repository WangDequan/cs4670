function btrees = runc(datas, classes, nrec, range, btreeso)
  if nargin < 4
    range = [1, size(datas, 1)];
  end
  t1 = cputime;
  btrees = cell(range(2), 1);
  parfor i=range(1):range(2)
    btrees{i} = tdidt(datas{i}, classes{i}, nrec, btreeso{i});
  end
  dt = cputime - t1;
  disp('Total time:');
  disp(dt);
end
