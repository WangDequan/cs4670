function [data, classes] = loadc(filepre, n)
  classes = cell(n,1);
  data = cell(n,1);
  disp('start');
  for i=1:n
    [data{i}, classes{i}] = readfile(strcat(filepre,num2str(i)));
    disp(i);
  end
  disp('end');
end
