function best = best_params(labels, traindata, v)
% 	best.v = v;
% 	best.acc = 0;
% 	p = struct('v',v,'t',0,'c',0,);
% 	for c = [0.01 0.05 0.1 0.5 1 5 10 50 100 500]
% 		% Try linear kernel
% 		p.t = 0;
% 		opts = sprintf('-v %d -t %d -c %d -q',v,p.t,p.c);
% 		p.acc = svmtrain(labels, traindata, opts);
% 		update(best,p);
% 		for g = []
% 		% Try polynomial kernel
% 		p.t = 1;
% 		for 

% function best = update(best, params)
% 	if params.acc > best.acc
% 		best = params;
% 	end
% end

	best.cv = 0;
	for log2c = -5:2:15,
	  for log2g = -15:2:3,
	    cmd = ['-v ', v, ' -c ', num2str(2^log2c), ' -g ', num2str(2^log2g)];
	    cv = svmtrain(labels, traindata, cmd);
	    if (cv >= bestcv),
	      best.cv = cv;
	      best.c = 2^log2c;
	      best.g = 2^log2g;
	    end
	    fprintf('%g %g %g (best c=%g, g=%g, rate=%g)\n', ...
	    	log2c, log2g, cv, bestc, bestg, bestcv);
	  end
	end
end