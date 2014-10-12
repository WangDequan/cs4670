function [hmatrix,alphamatrix] = trainboost(tclass, algopreds, iterations)
	% algopreds is a matrix of the predictions for each respective algorithm
	% the number of rows in algopreds is the number of algorithms to be boosted 
	% together and each row contains the predictions for each training example
	% tdata is the matrix containing all of the training vectors
	% tclass is the vector containing the true class of each training vector
  
	% iterations subject to change (might make it a param)
	% iterations = 10;
  
	% beta is a threshold for adaboost algorithm. probably won't matter with low 
	% number of iterations
	beta = .00000000000001;
  
	[numalgo,numfeat] = size(algopreds);
	%[~,numfeat] = size(tdata);
  
	% H is matrix containing the best algorithm
	H = zeros(iterations,1);
	Alphas = zeros(iterations,1);
	
	% D is the matrix of weighting for adaboost, ultimately calculates pred
	D = zeros((iterations+1),numfeat);
	D(1,:) = 1;
  
	for i=1:iterations
        
		errors = zeros(numalgo,1);
		for j=1:numalgo
			errors(j,1) = weightedError(tclass, algopreds(j,:), D(i,:), numfeat);
		end
	[bestErr, bestAlgo] = max(errors(:,1));
        
        errors = (abs(.5-errors));
      % [bestErr, bestAlgo] = max(errors(:,1));
        
		if abs(.5-bestErr)<beta
            disp('BREAKING');
            hmatrix = H;
            alphamatrix = Alphas;
			return
		end
		
		H(i,1) = bestAlgo;
		
		% alphai is a function which weights each iteration's legitimacy
        bestErr=1.0-bestErr;
		Alphas(i,1) =(.5*log((1.0-bestErr)/bestErr));%og(numalgo-1);
		%disp((Alphas(i,1)))
        
		correct = zeros(1,numfeat);
		correct(1,:) = (tclass(1,:)~=algopreds(bestAlgo,:));
		correct = exp((Alphas(i,1).*correct)-1.0);
		%(correct(1,1:3))
		D(i+1,:) = D(i,:).*correct;
        
        denom = sum(D(i+1,:))/numfeat;
        
		D(i+1,:) = D(i+1,:)/denom;
	end
 
	hmatrix = H;
	alphamatrix = Alphas;
 
end

function err = weightedError(tclass, algopred, currD, numfeat)
	correct = zeros(1,numfeat);
	correct(1,:) = (tclass(1,:)==algopred(1,:));
    
	correct(1,:) = correct(1,:).*currD(1,:);
    correct(1,:)=(correct(1,:)/numfeat);
	
	err = sum(correct(1,:));
    
end