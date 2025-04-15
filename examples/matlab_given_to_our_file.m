clear

fprintf('Loading grid data... \n');
grid_directory = '+data/';
%grid_directory = '';
%grid_file      = 'S433_Ekangen_Validation';
grid_file      = 'S495_Linghem_Validation';



% Load grid data
grid_data = load([grid_directory, grid_file]);

%open file

fd = fopen("graph.txt", "w");


% Now we have the whole structure and can generate the network:

%put out "grid"

fprintf(fd, "grid\n");

% for pair in grid_data.connectionBuses
    % put out the pair and get the Z_ser[i,j] ( or Y_shy[i.j] ? )

connections = grid_data.connectionBuses;

for k = 1:size(connections,1)
    i = connections(k,1);
    j = connections(k,2);
    z = grid_data.Z_ser(i,j);
    % We have to do - 1 since matlab is 1 indexed :(
    fprintf(fd,"%d %d (%f, %f)\n", i - 1, j - 1, real(z), imag(z));
end
% put %
fprintf(fd, "%%\n");

% get all index where gird_data.busisLoad == 1 
    % put them as load
is_load = grid_data.busIsLoad;

for k = 1:size(is_load,1)
    if is_load(k)
        % We have to do - 1 since matlab is 1 indexed :(
        fprintf(fd,"%d\n", k-1);
    end
end

    
% put %

fprintf(fd, "%%\n");

% put connections
fprintf(fd, "connections\n");

% put %
fprintf(fd, "%%");
fprintf("Graph converted! \n");

fclose(fd);

fprintf("Gathering variables...\n")
fd = fopen("variables.txt", "w");


U_bus = grid_data.U_bus;
S_bus = grid_data.S_bus;


fprintf(fd, "SBase: %f\n", grid_data.TransformerData.S_base);
fprintf(fd, "UBase ( prim, sec): %f %f\n", grid_data.TransformerData.U_prim_base, grid_data.TransformerData.U_sec_base);
fprintf(fd, "ZBase ( prim, sec): %f %f\n", grid_data.TransformerData.Z_prim_base, grid_data.TransformerData.Z_sec_base);
fprintf(fd, "IBase ( prim, sec): %f %f\n", grid_data.TransformerData.I_prim_base, grid_data.TransformerData.I_sec_base);

fprintf(fd, "U_bus\n");
for n = 1:size(U_bus,1)
    for m = 1:size(U_bus,2)
        fprintf(fd,"%f ",U_bus(n,m));
    end
    fprintf(fd,"\n");
end

fprintf(fd, "S_bus\n");
for n = 1:size(S_bus,1)
    for m = 1:size(S_bus,2)
        fprintf(fd,"%f ",S_bus(n,m));
    end
    fprintf(fd,"\n");
end

fclose(fd);