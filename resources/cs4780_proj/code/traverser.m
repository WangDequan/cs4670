function traverser(tree, data, class)
  classes = traverse(tree, data);
  t1 = cputime;
  acc = sum(classes == class) / size(data,1);
  t2 = cputime;
  disp('Time:');
  disp(t2-t1);
  disp('Accuracy:');
  disp(acc);
end
