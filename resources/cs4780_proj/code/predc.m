function resn = predc(btrees, data, buckets);
  n = size(data,1);
  n2 = size(buckets,1);
  assert(n == n2);

  resn = zeros(n,1);
  classes = unique(buckets);
  cnum = size(classes,1);
  for i=1:cnum
    disp(i);
    c = classes(i);
    subc = (buckets == c);
    res = traverse(btrees{c}, data(subc, :));
    resn(subc) = res;
  end
end
