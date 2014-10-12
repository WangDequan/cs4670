function [data, class] = readfile(filename)
% loads data from an svlight file
  t1 = cputime;
  fid = fopen(filename);
  disp('open');
  fcontent = textscan(fid,'%s','delimiter','\n');
  fclose(fid);
  disp('read');
  fcontent = cellfun(@(x) sscanf(x,'%g'),strrep(fcontent{1},':',' '),'UniformOutput',false);
  disp('extracted');
  rows = size(fcontent,1);
  if (rows > 0)
    colmax = max(cellfun(@(x) max(x(2:2:end)),fcontent(:)));
    disp('maxed');
    cols = colmax + 1;
    class = cellfun(@(x) x(1),fcontent);
    disp('classed');
    data = sparse(rows, cols);
    for i=1:rows
      data = data + sparse(i, fcontent{i}(2:2:end),fcontent{i}(3:2:end), rows, cols);
    end
    disp('looped');
  else
    disp('empty');
    data = []
    class = []
  end
  t2 = cputime;
  disp(t2-t1);
end
