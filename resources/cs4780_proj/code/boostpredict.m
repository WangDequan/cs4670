% hmat and amat are the respective matrices returned by trainboost
% algopreds is a matrix containing the prediction of each respective algorithm 
% for each instance we are trying to classify usings adaboost
% VERY IMPORTANT -- same algorithms in same order as trainboost for algopreds
function pred = boostpredict(hmat, amat, algopreds)
	[~,numpreds] = size(algopreds);
	[t,~] = size(hmat);
	pred = zeros(1,numpreds);
	numclasses = max(max(algopreds));
	
	for i=1:numpreds
		classweights = zeros(1,numclasses);
		for j=1:t
			% saver included only to clean up code
            if(hmat(j,1)~=0)
                
                
                %saver = classweights(1,hmat(j,1));
                classweights(1,hmat(j,1)) = classweights(1,hmat(j,1)) + amat(j,1);
            end
		end
		[~,currPred] = max(classweights(1,:));
        
		pred(1,i) = algopreds(currPred,i);
		
	end
	
end