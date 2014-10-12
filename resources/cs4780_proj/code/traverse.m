function classes = traverse(tree, data)
  if (numel(tree) == 1)
    classes = repmat(abs(tree), size(data, 1), 1);
  else
    classes = travtestrec(tree, data, 1);
  end
end

function classes = travtestrec(tree, data, row)
  if (row < 0)
    classes = repmat(abs(row), size(data,1), 1);
  else
    left = (data(:, tree(row,1)) < tree(row,2));
    right = ~left;
    classes = zeros(size(data,1),1);
    classes(left) = travtestrec(tree, data(left,:), tree(row,3));
    classes(right) = travtestrec(tree, data(right,:), tree(row,4));
  end
end
