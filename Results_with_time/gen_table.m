results=zeros(6,16);
results(1,:)=calc_normalized_score_new_all_v1('acadadvising',3);
results(2,:)=calc_normalized_score_new_all_v1('navigation',4);
results(3,:)=calc_normalized_score_new_all_v1('race',5);
results(4,:)=calc_normalized_score_new_all_v1('sailing',4);
results(5,:)=calc_normalized_score_new_all_v1('gol',5);
results(6,:)=calc_normalized_score_new_all_v1('sysadmin',5);
data=cell(6,8);
for i=1:6
    for j=1:8
        data(i,j)=cellstr(sprintf('%0.2f / %0.2f  ', results(i,j),results(i,j+8)));
    end
end
celldata=cellstr(data)