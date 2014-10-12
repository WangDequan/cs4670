function fix_sort(filename,outname)
	fin = fopen(filename,'r');
	fout = fopen(outname,'w');
	while ~feof(fin)
		line = strtrim(fgetl(fin));
		splits = strsplit(line, ' ');
		new_splits = char([splits(1) sort_nat(splits(2:length(splits)))]);
		for i = 1:length(new_splits)
			fprintf(fout,'%s ',strtrim(new_splits(i,:)));
		end
		fprintf(fout,'\n');
	end
	fclose(fin);
	fclose(fout);
end