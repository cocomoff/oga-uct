function calc_normalized_score_new(domain,count)
total_score=zeros(count+1,6);
%domain='acadadvising'
%domain='navigation'
%domain='race'
%domain='sysadmin'
for m=0:count
    if(m!=0)
        asap=dlmread(strcat('RESULTS_',domain,int2str(m),'/ALL/asapNew.txt'),' ');
        base=dlmread(strcat('RESULTS_',domain,int2str(m),'/ALL/baseNew.txt'),' ');
        ogak1=dlmread(strcat('RESULTS_',domain,int2str(m),'/ALL/k1New.txt'),' ');
        ogak3=dlmread(strcat('RESULTS_',domain,int2str(m),'/ALL/k3New.txt'),' ');
        ogak5=dlmread(strcat('RESULTS_',domain,int2str(m),'/ALL/k5New.txt'),' ');
        ogak7=dlmread(strcat('RESULTS_',domain,int2str(m),'/ALL/k7New.txt'),' ');
    else
        asap=dlmread(strcat('RESULTS_',domain,'/ALL/asapNew.txt'),' ');
        base=dlmread(strcat('RESULTS_',domain,'/ALL/baseNew.txt'),' ');
        ogak1=dlmread(strcat('RESULTS_',domain,'/ALL/k1New.txt'),' ');
    	ogak3=dlmread(strcat('RESULTS_',domain,'/ALL/k3New.txt'),' ');
    	ogak5=dlmread(strcat('RESULTS_',domain,'/ALL/k5New.txt'),' ');
    	ogak7=dlmread(strcat('RESULTS_',domain,'/ALL/k7New.txt'),' ');
	end
    temp=zeros(6,1);
    temp(6,1)=ogak7(size(ogak7,1),4);
    temp(5,1)=ogak5(size(ogak5,1),4);
    temp(4,1)=ogak3(size(ogak3,1),4);
    temp(3,1)=ogak1(size(ogak1,1),4);
    temp(2,1)=asap(size(asap,1),4);
    temp(1,1)=base(size(base,1),4);
   
    max_time=min(temp);
    asapReg=zeros(6,1);
    baseReg=zeros(6,1);
    ogak1Reg=zeros(6,1);
    ogak3Reg=zeros(6,1);
   	ogak5Reg=zeros(6,1);
   	ogak7Reg=zeros(6,1);
    for i=1:6
        val=i*max_time/6;
        for j=2:size(asap,1)
            if(val<asap(j,4))
                break;
            end
        end
        asapReg(i,1)=asap(j-1,2)+(val-asap(j-1,4))*(asap(j,2)-asap(j-1,2))/(asap(j,4)-asap(j-1,4));
        for j=2:size(base,1)
            if(val<base(j,4))
                break;
            end
        end
        baseReg(i,1)=base(j-1,2)+(val-base(j-1,4))*(base(j,2)-base(j-1,2))/(base(j,4)-base(j-1,4));
        for j=2:size(ogak1,1)
            if(val<ogak1(j,4))
                break;
            end
        end
        ogak1Reg(i,1)=ogak1(j-1,2)+(val-ogak1(j-1,4))*(ogak1(j,2)-ogak1(j-1,2))/(ogak1(j,4)-ogak1(j-1,4));
        for j=2:size(ogak3,1)
            if(val<ogak3(j,4))
                break;
            end
        end
        ogak3Reg(i,1)=ogak3(j-1,2)+(val-ogak3(j-1,4))*(ogak3(j,2)-ogak3(j-1,2))/(ogak3(j,4)-ogak3(j-1,4));
        for j=2:size(ogak5,1)
            if(val<ogak5(j,4))
                break;
            end
        end
        ogak5Reg(i,1)=ogak5(j-1,2)+(val-ogak5(j-1,4))*(ogak5(j,2)-ogak5(j-1,2))/(ogak5(j,4)-ogak5(j-1,4));
        for j=2:size(ogak7,1)
            if(val<ogak7(j,4))
                break;
            end
        end
        ogak7Reg(i,1)=ogak7(j-1,2)+(val-ogak7(j-1,4))*(ogak7(j,2)-ogak7(j-1,2))/(ogak7(j,4)-ogak7(j-1,4));
    end
    score_mat=zeros(6,6);
    score_mat(:,1)=baseReg(:,1);
    score_mat(:,2)=asapReg(:,1);
    score_mat(:,3)=ogak1Reg(:,1);
    score_mat(:,4)=ogak3Reg(:,1);
    score_mat(:,5)=ogak5Reg(:,1);
    score_mat(:,6)=ogak7Reg(:,1);
    min_value=min(min(score_mat(:,:)));
    max_value=max(max(score_mat(:,:)));
    for i=1:6
        for j=1:6
            score_mat(i,j)=(score_mat(i,j)-max_value)/(min_value-max_value);
        end
    end
    
    total_score(m+1,1)=sum(score_mat(:,1));
    total_score(m+1,2)=sum(score_mat(:,2));
    total_score(m+1,3)=sum(score_mat(:,3));
    total_score(m+1,4)=sum(score_mat(:,4));
    total_score(m+1,5)=sum(score_mat(:,5));
    total_score(m+1,6)=sum(score_mat(:,6));
end
total_score=total_score/6;
norm_score=mean(total_score,1);
sdev=std(total_score,1);
sdev=2*sdev/sqrt(count+1)
norm_score=norm_score
       
            
                
                
        

    
