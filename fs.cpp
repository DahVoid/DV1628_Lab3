#include <iostream>
#include "fs.h"
#include <cstring>
#include <vector>
#include <typeinfo> // for debugging
using namespace std; // Check if used in datorsalen

struct dir_entry dir_entries[ROOT_SIZE];
char char_dir_path[1][56] = {""}; // gör om till en lista av index emot dir_entries.
std::vector<std::string> dir_path{""};
char curr_dir_content[64][56]; // we're already limited to 64 dir_entries 

void init_dir_content(vector path) {
  // Is root folder
  if(path.size() == 1){
    // find all root dir_entries
    for(int i = 0; i < ROOT_SIZE; ++i){ 

    }

  } else {
    // Is not root folder
  }

}

FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
    // Init ROOT_BLOCK on boot
    disk.read(ROOT_BLOCK, (uint8_t*)dir_entries);

    // Init FAT
    disk.read(FAT_BLOCK, (uint8_t*)fat);

    // Confirm that read properly reads the FAT block
    int checkIfDataSaved = fat[ROOT_BLOCK];
    cout << "-- Finished booting -- \n";

}

FS::~FS()
{

}

// formats the disk, i.e., creates an empty file system
int
FS::format()
{
    string empty_str = "";
    uint8_t* empty_block = (uint8_t*)empty_str.c_str();

    for (int i = 2; i < disk.get_no_blocks(); i++)
    {
      fat[i] = FAT_FREE;
      disk.write(i, empty_block);
    }

    for(int i = 0; i < ROOT_SIZE; i++) {

      struct dir_entry temp_entry;
      strcpy(temp_entry.file_name, "");
      temp_entry.size = 0;
      temp_entry.first_blk = 0;
      temp_entry.type = 0;
      temp_entry.access_rights = 0;
      dir_entries[i] = temp_entry;
    }

    fat[ROOT_BLOCK] = FAT_EOF;
    disk.write(ROOT_BLOCK, (uint8_t*)&dir_entries); // dir_entries in the file

    fat[FAT_BLOCK] = FAT_EOF;
    disk.write(FAT_BLOCK, (uint8_t*)&fat); // Fat in the file
    std::cout << "FS::format()\n";
    return 0;
}

// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int
FS::create(std::string filepath)
{
    //get file size
    int size_of_file;
    string string_to_eval;
    string string_to_eval_temp;
    int start_block;

    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if (dir_entries[i].file_name == filepath)
      {
        cout << "A file with that name already exists in this directory, try again\n";
        return 0;
      }
    }

    cout << "Enter the information: ";

    while (getline(cin, string_to_eval_temp) && string_to_eval_temp.length() != 0)
    {
      string_to_eval += string_to_eval_temp + "\n";
    }
    // convert string_to_eval to uint8_t*
    uint8_t* block = (uint8_t*)string_to_eval.c_str();

    size_of_file = string_to_eval.length();
    int size_of_file_temp = size_of_file;

    //insert data into directory entry

    // check amount of block
    int num_blocks = size_of_file / BLOCK_SIZE + 1;

    int free_spaces[num_blocks];
    int free_space_counter = 0;
    for (int i = 0; i < BLOCK_SIZE/2; i++) {
      // check where the file can fit in the fat
      if (fat[i] == FAT_FREE) {
        free_spaces[free_space_counter] = i;
        free_space_counter++;
      }

      if (free_space_counter >= num_blocks) {
        // go back and fill
        start_block = i - free_space_counter + 1;

        //set fat values
        int elements_in_fat_counter = 1;
        for(int j = start_block; j < start_block + num_blocks; j++) {
          if(elements_in_fat_counter == free_space_counter) {
            fat[j] = FAT_EOF;
            elements_in_fat_counter++;
          } else {
            fat[j] = free_spaces[elements_in_fat_counter];
            elements_in_fat_counter++;
          }
        }
        disk.write(FAT_BLOCK, (uint8_t*)&fat);

        // Write to the disk
        for(int j = start_block; j < start_block + num_blocks; j++) {
          if (num_blocks == 1)
          {
            disk.write(j, block);
            break;
          }
          for (int r = 0; r < num_blocks-1; r++)
          {
            string block_to_write = string_to_eval.substr(r*BLOCK_SIZE, r*BLOCK_SIZE + BLOCK_SIZE);
            uint8_t* block = (uint8_t*)block_to_write.c_str();
            disk.write(j+r, block);
            size_of_file_temp = size_of_file_temp - BLOCK_SIZE;
          }
          string block_to_write = string_to_eval.substr((num_blocks-1)*BLOCK_SIZE, (num_blocks-1)*BLOCK_SIZE + size_of_file_temp);
          uint8_t* block = (uint8_t*)block_to_write.c_str();
          disk.write(j+num_blocks-1, block);
          break;
        }
        break;
      }
    }

    struct dir_entry temp_entry;

    strcpy(temp_entry.file_name, filepath.c_str());

    temp_entry.size = size_of_file;
    temp_entry.first_blk = start_block;
    temp_entry.type = TYPE_FILE;
    temp_entry.access_rights = READ;

    // dir_entry to root
    for(int i = 0; i < ROOT_SIZE; i++) {
      if(dir_entries[i].first_blk == 0) {
        dir_entries[i] = temp_entry;
        break;
      }

    }

    disk.write(ROOT_BLOCK, (uint8_t*)&dir_entries);

    std::cout << "FS::create(" << filepath << ")\n";
    return 0;
}


// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath)
{
    std::cout << "FS::cat(" << filepath << ")\n";
    int blocks_to_read;
    int entry_index;
    int file_found = 0;

    // Find file
    for(int i = 0; i < ROOT_SIZE; i++){

      if(dir_entries[i].file_name == filepath){
        file_found = 1;
        entry_index = i;
        blocks_to_read = dir_entries[i].size/BLOCK_SIZE;
        // Add rest block if exists
        if(dir_entries[i].size%BLOCK_SIZE > 0) {
          blocks_to_read++;
        }
        break;
      }
    }

    if (file_found == 0)
    {
      cout << "No file with that name found, try again\n";
      return 0;
    }

    if(blocks_to_read == 0){
      return -1;
    }

    // read from disk
    char block_content[BLOCK_SIZE];
    char read_data[BLOCK_SIZE*blocks_to_read];
    memset(read_data, 0, BLOCK_SIZE*blocks_to_read);
    int next_block = dir_entries[entry_index].first_blk;
    for(int i = 0; i < blocks_to_read; i++) {
      disk.read(next_block, (uint8_t*)block_content);
      strcat(read_data, block_content);

      if(next_block != -1){
        next_block = fat[next_block];
      }
    }

    //Present content
    cout << read_data << endl;
    return 0;
}

// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
  for (int i = 0; i < ROOT_SIZE; i++)
      {
        if (dir_entries[i].first_blk != 0)
        {
          cout << dir_entries[i].file_name << "                     " << dir_entries[i].size << "\n";
        }
      }

    return 0;
}

// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int
FS::cp(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";
    int dir_entry_index = -1;
    char name_array[56];

    for (int i = 0; i < ROOT_SIZE; i++)
    {
      if (dir_entries[i].first_blk != 0)
      {
        strcpy(name_array, sourcepath.c_str());

        if (dir_entries[i].file_name == sourcepath)
        {
          dir_entry_index = i;
          break;
        }
      }
    }

    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if (dir_entries[i].file_name == destpath)
      {
        cout << "A file with that name already exists in this directory, try again\n";
        return 0;
      }
    }

    if (dir_entry_index == -1)
    {
      cout << "No file with that name found\n";
      return 0;
    }

    cout << "Size of file in dir: " << dir_entries[dir_entry_index].size << endl;

    int blocks_to_read = dir_entries[dir_entry_index].size/BLOCK_SIZE;
    if(dir_entries[dir_entry_index].size%BLOCK_SIZE > 0)
    {
      blocks_to_read++;
    }

    //GET FILE information

    uint32_t size_of_file = dir_entries[dir_entry_index].size;
    uint16_t first_block = dir_entries[dir_entry_index].first_blk;
    uint8_t type = dir_entries[dir_entry_index].type;
    uint8_t access_rights = dir_entries[dir_entry_index].access_rights;

    cout << "size of file:  "<< size_of_file << endl;
    cout << "block to read:  "<< blocks_to_read << endl;
    //Get data from file
    //FEEEEEEEEEL start här
    char* block_content;
    block_content = (char*)calloc(BLOCK_SIZE, sizeof(char));
    char* read_data;
    read_data = (char*) calloc(blocks_to_read,BLOCK_SIZE);
    int next_block = dir_entries[dir_entry_index].first_blk;
    for(int i = 0; i < blocks_to_read; i++)
    {
      disk.read(next_block, (uint8_t*)block_content);
      cout << block_content << "\n";
      strcat(read_data, block_content);

      if(next_block != -1)
      {
        next_block = fat[next_block];
      }
    }
    cout << read_data << "\n";

    //**''*''*'''*''*''**CREATE FILE copied from CREATE function**''*''*''*

    //get file size
    int start_block;
    string string_to_eval = read_data;
    free(read_data);
    // convert string_to_eval to uint8_t*
    uint8_t* block = (uint8_t*)string_to_eval.c_str();

    size_of_file = string_to_eval.length();
    int size_of_file_temp = size_of_file;

    //FEEEEEEEEEL end här

    //insert data into directory entry

    // check amount of block

    // int num_blocks = size_of_file / BLOCK_SIZE + 1;
    // cout << "num of blocks: "<< num_blocks << endl;
    cout << "size of file:  "<< size_of_file << endl;
    cout << "BLOCK_SIZE:  "<< BLOCK_SIZE << endl;
    int free_spaces[blocks_to_read];
    int free_space_counter = 0;
    for (int i = 0; i < BLOCK_SIZE/2; i++) {
      // check where the file can fit in the fat
      if (fat[i] == FAT_FREE) {
        free_spaces[free_space_counter] = i;
        free_space_counter++;
      }

      if (free_space_counter >= blocks_to_read) {
        // go back and fill
        start_block = i - free_space_counter + 1;

        int elements_in_fat_counter = 1;
        for(int j = start_block; j < start_block + blocks_to_read; j++) {
          if(elements_in_fat_counter == free_space_counter) {
            fat[j] = FAT_EOF;
            elements_in_fat_counter++;
          } else {
            fat[j] = free_spaces[elements_in_fat_counter];
            elements_in_fat_counter++;
          }
        }

        for(int j = start_block; j < start_block + blocks_to_read; j++) {
          if (blocks_to_read == 1)
          {
            disk.write(j, block);
            break;
          }
          for (int r = 0; r < blocks_to_read-1; r++)
          {
            string block_to_write = string_to_eval.substr(r*BLOCK_SIZE, r*BLOCK_SIZE + BLOCK_SIZE);
            uint8_t* block = (uint8_t*)block_to_write.c_str();
            disk.write(j+r, block);
            size_of_file_temp = size_of_file_temp - BLOCK_SIZE;
          }
          string block_to_write = string_to_eval.substr((blocks_to_read-1)*BLOCK_SIZE, (blocks_to_read-1)*BLOCK_SIZE + size_of_file_temp);
          uint8_t* block = (uint8_t*)block_to_write.c_str();
          disk.write(j+blocks_to_read-1, block);
          break;
        }
        break;
      }
    }
    struct dir_entry temp_entry;

    strcpy(temp_entry.file_name, destpath.c_str());

    temp_entry.size = size_of_file;
    temp_entry.first_blk = start_block;
    temp_entry.type = type;
    temp_entry.access_rights = access_rights;

    // entry to root
    for(int i = 0; i < ROOT_SIZE; i++) {
      if(dir_entries[i].first_blk == 0) {
        dir_entries[i] = temp_entry;
        break;
      }
    }

    disk.write(ROOT_BLOCK, (uint8_t*)&dir_entries); // dir_entries in the file
    disk.write(FAT_BLOCK, (uint8_t*)&fat); // Fat in the file
    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
    // Is destpath a directory?
    for(int i = 0; i < ROOT_SIZE; i++){
      if(dir_entries[i].file_name == destpath && dir_entries[i].type == TYPE_DIR){
        cout << "TODO Move to directory \n" << endl;
        return 0;
      }
    }

    // find source file
    for(int i = 0; i < ROOT_SIZE; i++){
      if(dir_entries[i].file_name == sourcepath){
        strcpy(dir_entries[i].file_name, destpath.c_str());
        disk.write(ROOT_BLOCK, (uint8_t*)&dir_entries); // dir_entries in the file

        return 0;
      }
    }

    cout << "No file with the name: " << sourcepath.c_str() << "\n";
    return 0;
}

// rm <filepath> removes / deletes the file <filepath>
int
FS::rm(std::string filepath)
{
    std::cout << "FS::rm(" << filepath << ")\n";

    int name_found = 0;

    for(int i = 0; i < ROOT_SIZE; i++){
      if(dir_entries[i].file_name == filepath){
        name_found = 1;
        // Clear fat
        int blocks_to_read = dir_entries[i].size/BLOCK_SIZE;
        if(dir_entries[i].size%BLOCK_SIZE > 0) {
          blocks_to_read++;
        }

        int curr_block = dir_entries[i].first_blk;
        int index = curr_block;
        for (int i = 0; i < blocks_to_read; i++) {
          index = curr_block;
          if(curr_block != -1){
          curr_block = fat[curr_block];
          }
          fat[index] = FAT_FREE;
        }

        // Clear dir entry
        struct dir_entry temp_entry;
        strcpy(temp_entry.file_name, "");
        temp_entry.size = 0;
        temp_entry.first_blk = 0;
        temp_entry.type = 0;
        temp_entry.access_rights = 0;
        dir_entries[i] = temp_entry;
      }
    }
    if (name_found == 0)
    {
      cout << "No file with that name found, try again\n";
      return 0;
    }

    disk.write(ROOT_BLOCK, (uint8_t*)&dir_entries); // dir_entries in the file
    disk.write(FAT_BLOCK, (uint8_t*)&fat); // Fat in the file

    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int
FS::append(std::string filepath1, std::string filepath2)
{
    std::cout << "FS::append(" << filepath1 << "," << filepath2 << ")\n";

    // *""*""*""*""*"" FIND FILES *""*""*""*""*""**
    //File 1
    int dir_entry_index_1 = -1;
    char name_array_1[56];

    for (int i = 0; i < ROOT_SIZE; i++)
    {
      if (dir_entries[i].first_blk != 0)
      {
        strcpy(name_array_1, filepath1.c_str());

        if (dir_entries[i].file_name == filepath1)
        {
          dir_entry_index_1 = i;
          break;
        }
      }
    }

    if (dir_entry_index_1 == -1)
    {
      cout << "No file with that name found\n";
      return 0;
    }

    int blocks_to_read_1 = dir_entries[dir_entry_index_1].size/BLOCK_SIZE;
    if(dir_entries[dir_entry_index_1].size%BLOCK_SIZE > 0)
    {
      blocks_to_read_1++;
    }

    //File 2
    int dir_entry_index_2 = -1;
    char name_array_2[56];

    for (int i = 0; i < ROOT_SIZE; i++)
    {
      if (dir_entries[i].first_blk != 0)
      {
        strcpy(name_array_2, filepath2.c_str());

        if (dir_entries[i].file_name == filepath2)
        {
          dir_entry_index_2 = i;
          break;
        }
      }
    }

    if (dir_entry_index_2 == -1)
    {
      cout << "No file with that name found\n";
      return 0;
    }

    int blocks_to_read_2 = dir_entries[dir_entry_index_2].size/BLOCK_SIZE;
    if(dir_entries[dir_entry_index_2].size%BLOCK_SIZE > 0)
    {
      blocks_to_read_2++;
    }

    // *""*""*""*""*"" READ FILE INFO *""*""*""*""*""**
    //file 1
    uint32_t size_of_file_1 = dir_entries[dir_entry_index_1].size;
    uint16_t first_block_1 = dir_entries[dir_entry_index_1].first_blk;
    uint8_t type_1 = dir_entries[dir_entry_index_1].type;
    uint8_t access_rights_1 = dir_entries[dir_entry_index_1].access_rights;

    //file_2
    uint32_t size_of_file_2 = dir_entries[dir_entry_index_2].size;
    uint16_t first_block_2 = dir_entries[dir_entry_index_2].first_blk;
    uint8_t type_2 = dir_entries[dir_entry_index_2].type;
    uint8_t access_rights_2 = dir_entries[dir_entry_index_2].access_rights;

    // *""*""*""*""*"" READ DATA FROM FILES *""*""*""*""*""**
    //file 1
    char block_content_1[BLOCK_SIZE];
    char read_data_1[BLOCK_SIZE*blocks_to_read_1];
    int next_block_1 = dir_entries[dir_entry_index_1].first_blk;
    for(int i = 0; i < blocks_to_read_1; i++)
    {
      disk.read(next_block_1, (uint8_t*)block_content_1);
      strcat(read_data_1, block_content_1);

      if(next_block_1 != -1)
      {
        next_block_1 = fat[next_block_1];
      }
    }

    //file 2
    char block_content_2[BLOCK_SIZE];
    char read_data_2[BLOCK_SIZE*blocks_to_read_2];
    int next_block_2 = dir_entries[dir_entry_index_2].first_blk;
    for(int i = 0; i < blocks_to_read_2; i++)
    {
      disk.read(next_block_2, (uint8_t*)block_content_2);
      strcat(read_data_2, block_content_2);

      if(next_block_2 != -1)
      {
        next_block_2 = fat[next_block_2];
      }
    }

    // *""*""*""*""*"" APPEND DATA *""*""*""*""*""**
    strcat(read_data_2, read_data_1);

    // *""*""*""*""*"" REMOVE FILE 2 *""*""*""*""*""**
    rm(filepath2);

    // *""*""*""*""*"" ADD NEW FILE 2 *""*""*""*""*""**

    //get file size
    int start_block;
    string string_to_eval = read_data_2;
    // convert string_to_eval to uint8_t*
    uint8_t* block = (uint8_t*)string_to_eval.c_str();

    int size_of_file = string_to_eval.length();
    int size_of_file_temp = size_of_file;

    //insert data into directory entry

    // check amount of block
    int num_blocks = size_of_file / BLOCK_SIZE + 1;

    int free_spaces[num_blocks];
    int free_space_counter = 0;
    for (int i = 0; i < BLOCK_SIZE/2; i++) {
      // check where the file can fit in the fat
      if (fat[i] == FAT_FREE) {
        free_spaces[free_space_counter] = i;
        free_space_counter++;
      }

      if (free_space_counter >= num_blocks) {
        // go back and fill
        start_block = i - free_space_counter + 1;

        int elements_in_fat_counter = 1;
        for(int j = start_block; j < start_block + num_blocks; j++) {
          cout << elements_in_fat_counter << " == "<< free_space_counter; // Breaks pga break nedan i loopen.
          if(elements_in_fat_counter == free_space_counter) {
            fat[j] = FAT_EOF;
            elements_in_fat_counter++;
          } else {
            fat[j] = free_spaces[elements_in_fat_counter];
            elements_in_fat_counter++;
          }
        }

        for(int j = start_block; j < start_block + num_blocks; j++) {
          if (num_blocks == 1)
          {
            disk.write(j, block);
            break;
          }
          for (int r = 0; r < num_blocks-1; r++)
          {
            string block_to_write = string_to_eval.substr(r*BLOCK_SIZE, r*BLOCK_SIZE + BLOCK_SIZE);
            uint8_t* block = (uint8_t*)block_to_write.c_str();
            disk.write(j+r, block);
            size_of_file_temp = size_of_file_temp - BLOCK_SIZE;
            cout << size_of_file_temp << "\n";
          }
          string block_to_write = string_to_eval.substr((num_blocks-1)*BLOCK_SIZE, (num_blocks-1)*BLOCK_SIZE + size_of_file_temp);
          uint8_t* block = (uint8_t*)block_to_write.c_str();
          disk.write(j+num_blocks-1, block);
          break;
        }
        break;
      }
    }
    struct dir_entry temp_entry;

    strcpy(temp_entry.file_name, filepath2.c_str());

    temp_entry.size = size_of_file;
    temp_entry.first_blk = start_block;
    temp_entry.type = type_2;
    temp_entry.access_rights = access_rights_2;

    // entry to root
    for(int i = 0; i < ROOT_SIZE; i++) {
      if(dir_entries[i].first_blk == 0) {
        dir_entries[i] = temp_entry;
        break;
      }
    }
    disk.write(ROOT_BLOCK, (uint8_t*)&dir_entries);  // Can't find this on disk after writing?

    return 0;
}

int
FS::mkdir(std::string dirpath)
{
    std::cout << "FS::mkdir(" << dirpath << ")\n";
    int start_block;
    int num_blocks = 1;

    int free_spaces[num_blocks];
    int free_space_counter = 0;

      for (int i = 0; i < BLOCK_SIZE/2; i++) {
        // check where the file can fit in the fat
        if (fat[i] == FAT_FREE) {
          free_spaces[free_space_counter] = i;
          free_space_counter++;
        }

        if (free_space_counter >= num_blocks) {
          // go back and fill
          start_block = i - free_space_counter + 1;
          
          //set fat values
          int elements_in_fat_counter = 1;
          for(int j = start_block; j < start_block + num_blocks; j++) {
            cout << elements_in_fat_counter << " == "<< free_space_counter; //
            if(elements_in_fat_counter == free_space_counter) {
              fat[j] = FAT_EOF;
              elements_in_fat_counter++;
            } else {
              fat[j] = free_spaces[elements_in_fat_counter];
              elements_in_fat_counter++;
            }
          }
          disk.write(FAT_BLOCK, (uint8_t*)&fat);  

          // write content list to disk
          
          char temp_dir_content[64][56] = {".."}; //We're already limited to 64 entries.
          uint8_t* block = (uint8_t*)temp_dir_content;
          disk.write(start_block, block);

          // write dir entry list
          struct dir_entry temp_entry;

          strcpy(temp_entry.file_name, dirpath.c_str());

          temp_entry.size = 0;
          temp_entry.first_blk = start_block;
          temp_entry.type = TYPE_DIR;
          temp_entry.access_rights = READ;

          // dir_entry to root block
          //find empty slot in dir entries
          for(int i = 0; i < ROOT_SIZE; i++) {
            if(dir_entries[i].first_blk == 0) {
              dir_entries[i] = temp_entry;
              break;
            }

          }
          disk.write(ROOT_BLOCK, (uint8_t*)&dir_entries); 
          break;
        }
      }
    return 0;
    
}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int
FS::cd(std::string new_dir)
{
    std::cout << "FS::cd(" << new_dir << ")\n";
// if new directory is ".." then we go up one level.
  if (new_dir == "..") {
    if(dir_path.size() == 1){
      cout << "Can't go futher up, you're at root. \n";
      return -1;
    }

    dir_path.resize(dir_path.size() - 1);

    // get new dir content
    return 0;
  } else {
    dir_path.push_back(new_dir);
     // get new dir content
    return 0;
  }
    cout << "directory not found" << endl;
    return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int
FS::pwd()
{
    std::cout << "FS::pwd()\n";
    string output_str = "";
    string temp;
    if(dir_path.size() > 1 ) {
      for(int i = 1; i < dir_path.size(); i++) {
        temp += dir_path[i];
        output_str.append("/" + temp);
      }

    } else {
      output_str.append("/");
    }

    
    cout << output_str << std::endl;
    return 0;
}

// chmod <accessrights> <filepath> changes the access rights for the
// file <filepath> to <accessrights>.
int
FS::chmod(std::string accessrights, std::string filepath)
{
    std::cout << "FS::chmod(" << accessrights << "," << filepath << ")\n";
    for (int i = 0; i < 64 ; i++){
      cout << "fat[" << i << "] = " << fat[i] << "\n";
    }

    return 0;
}
