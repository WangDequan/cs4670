function [ ] = plot_pr(plot_title, varargin)
% SD_PLOT_PR Plot multiple precision recall curves saved in .pr
% files
%
% USAGE:
%     plot_pr("Plot Title", 'file1.pr', 'File 1 Legend', 'file2.pr', 'File 2 legend', ...)

output_fname = '';

figure;
cla;
hold all

xlim([0, 1]);
ylim([0, 1]);

pr_names = {};

%set(gca,'LineStyleOrder',{'-','--', '-x', '-v', '-s', '-+', '-o'});
%colors = [10, 98, 16; 50, 99, 198; 220, 57, 18; 255, 153, 0]/255;  %hsv2rgb(cat(2, linspace(0,1,n_color)', 0.9 * ones(n_color, 1), 0.85 * ones(n_color, 1)));

linestyles = {'x', 'v', '*', 'o','+'};
set(gca,'LineStyleOrder',linestyles);
%n_color = 5;

%n_color = 5;
colors = [10, 98, 16; 50, 99, 198; 220, 57, 18; 255, 153, 0]/255;  %hsv2rgb(cat(2, linspace(0,1,n_color)', 0.9 * ones(n_color, 1), 0.85 * ones(n_color, 1)));
%colors = [10, 98, 16; 50, 99, 198; 255, 153, 0; 220, 57, 18]/255;  %hsv2rgb(cat(2, linspace(0,1,n_color)', 0.9 * ones(n_color, 1), 0.85 * ones(n_color, 1)));
set(gca,'ColorOrder', colors);%(1:end - 1, :));

%set(gca,'ColorOrder', [0 0 0]);

plot_num = 1;
for k = 1:2:length(varargin)
    switch varargin{k}
        case 'output'
            output_fname = varargin{k + 1};

        otherwise
            % it's pair of the form (pr_fname, pr_name)
            pr_fname = varargin{k};
            pr_name = varargin{k + 1};
            
            pr_names{end + 1} = pr_name;
                        
            if ~exist(pr_fname, 'file')
                sd_log(1, sprintf('WARNING: Skipping non-existing file %s', pr_fname));
                plot(0, 0);
                continue;
            end
            
            pr =  dlmread(pr_fname, ' ', 1, 0);                        
                
            if size(pr, 1) == 0
                fprintf('WARNING: Skipping empty file %s', deteval_fname);
                plot(0, 0);
                continue
            end
            
            X = pr(:,2);
            Y = pr(:,1);
            
            [X, idxs] = sort(X);
            Y = Y(idxs);
            
            all_idxs = (1:length(Y))';
            for i = 2:length(Y)
                sel = all_idxs < i & Y < Y(i);
                if nnz(sel) > 0
                    Y(sel) = Y(i);
                end
            end
            
            
            %plot(X, Y, 'LineWidth', 1.5);
            
            sel = linspace(plot_num/100.0, 1, 10);
            for ii = 1:length(sel)
                sel(ii) = find(sel(ii) <= X, 1 );
            end
            
            plot(X, Y, '-', 'Color', colors(plot_num,:), 'LineWidth', 2);
            %plot(X(sel), Y(sel), linestyles{plot_num}, 'Color', colors(plot_num,:), 'MarkerFaceColor', [1 1 1]);

            plot_num = plot_num + 1;
    end
end

title(plot_title);
xlabel('Recall');
ylabel('Precision');
legend(pr_names, 'Location', 'SouthWest');
legend('boxoff');
daspect([1 1 1]);

if strcmp(output_fname, '') == 0
    [p n e] = fileparts(output_fname);
    if strcmp(e, '.eps')
        saveas(gcf, output_fname, 'epsc');
    else
        saveas(gcf, output_fname);
    end
end


end