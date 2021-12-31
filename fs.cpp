#include <iostream>
#include "fs.h"
#include <cstring>
#include <vector>
#include <typeinfo> // for debugging
using namespace std; // Check if used in datorsalen

struct dir_entry dir_entries[ROOT_SIZE];

FS::FS()
{ 
    std::cout << "FS::FS()... Creating file system\n";
    // Init ROOT_BLOCK on boot
    disk.read(ROOT_BLOCK, (uint8_t*)dir_entries);
    cout << "found dirs on boot: " << sizeof(dir_entries)/ROOT_SIZE << "\n";

    // Init FAT 
    disk.read(FAT_BLOCK, (uint8_t*)fat);

    // Confirm that read properly reads the FAT block
    int checkIfDataSaved = fat[ROOT_BLOCK];
    cout << "fat[0] should be 1 but is: " << checkIfDataSaved << "\n";
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
    bool no_block = false;
    int size_of_file;
    string string_to_eval;
    string string_to_eval_temp;
    int start_block;
    cout << "Enter the information: ";

    while (getline(cin, string_to_eval_temp) && string_to_eval_temp.length() != 0)
    {
      string_to_eval += string_to_eval_temp + "\n";
    }
    // convert string_to_eval to uint8_t*
    uint8_t* block = (uint8_t*)string_to_eval.c_str();


    size_of_file = string_to_eval.length();
    int size_of_file_temp = size_of_file;

    cout << string_to_eval;
    //insert data into directory entry

    // check amount of block
    int num_blocks = size_of_file / BLOCK_SIZE + 1;

    // Always 1 atleast
    // if(num_blocks == 0) {
    //   num_blocks = 1;
    //   bool no_block = true;
    // }

    cout << "num blocks: " << num_blocks << "\n";
    int free_spaces[num_blocks];
    int free_space_counter = 0;
    cout << "Start for loop\n";
    for (int i = 0; i < BLOCK_SIZE/2; i++) {
      // check where the file can fit in the fat
      if (fat[i] == FAT_FREE) {
        cout << "Free space at: " << i << "\n";
        free_spaces[free_space_counter] = i;
        free_space_counter++;
      }

      if (free_space_counter >= num_blocks) {
        cout << "Found space\n";
        // go back and fill
        start_block = i - free_space_counter + 1;
        cout << "Start block: " << start_block << "\n";
        
        //set fat values
        int elements_in_fat_counter = 1;
        for(int j = start_block; j < start_block + num_blocks; j++) {
          cout << elements_in_fat_counter << " == "<< free_space_counter; //
          if(elements_in_fat_counter == free_space_counter) {
            cout << "Setting EOF \n";
            fat[j] = FAT_EOF;
            elements_in_fat_counter++;
          } else {
            cout << "Adding element in fat " << free_spaces[elements_in_fat_counter-1] << "\n";
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
            cout << "filling current"<< "\n";
            string block_to_write = string_to_eval.substr(r*BLOCK_SIZE, r*BLOCK_SIZE + BLOCK_SIZE);
            uint8_t* block = (uint8_t*)block_to_write.c_str();
            disk.write(j+r, block);
            size_of_file_temp = size_of_file_temp - BLOCK_SIZE;
            cout << size_of_file_temp << "\n";
          }
          cout << "filling leftover"<< "\n";
          string block_to_write = string_to_eval.substr((num_blocks-1)*BLOCK_SIZE, (num_blocks-1)*BLOCK_SIZE + size_of_file_temp);
          uint8_t* block = (uint8_t*)block_to_write.c_str();
          disk.write(j+num_blocks-1, block);
          cout << "writing to disk\n";
          break;
        }

        cout << "finished to write \n";
        break;
      }

    }

    cout << "End loop\n";

    struct dir_entry temp_entry;

    strcpy(temp_entry.file_name, filepath.c_str());

    temp_entry.size = size_of_file;
    temp_entry.first_blk = start_block;
    temp_entry.type = TYPE_FILE;
    temp_entry.access_rights = READ;

    // dir_entry to root
    for(int i = 0; i < ROOT_SIZE; i++) {
      if(dir_entries[i].first_blk == 0) {
        cout << "Empty dir slot in dir array, putting temp array there\n";
        dir_entries[i] = temp_entry;
        break;
      }

    }

    cout << "size of temp_entry: " << sizeof(temp_entry) << "\n";
    cout << "entry_dir lenght: " << sizeof(dir_entries)/ROOT_SIZE << "\n";
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


    // Find file
    cout << "Finding file \n";
    for(int i = 0; i < ROOT_SIZE; i++){

      if(dir_entries[i].file_name == filepath){
        entry_index = i;
        blocks_to_read = dir_entries[i].size/BLOCK_SIZE;
        // Add rest block if exists
        if(dir_entries[i].size%BLOCK_SIZE > 0) {
          blocks_to_read++;
        }
        break;
      }
    }

    if(blocks_to_read == 0){
      cout << "Can't find file \n";
      return -1;
    }
  
    // read from disk
    cout << "read from disk\n";
    cout << sizeof(dir_entries)/sizeof(struct dir_entry) << "\n";
    cout << "blocks_to_read: " << blocks_to_read << "\n";
    cout << "entry index: " << entry_index << "\n";
    cout << "Test" << dir_entries[0].first_blk << endl;
    
    char block_content[BLOCK_SIZE];
    char read_data[BLOCK_SIZE*blocks_to_read];
    int next_block = dir_entries[entry_index].first_blk;
    for(int i = 0; i < blocks_to_read; i++) {
      cout << "reading entry " << next_block << endl;
      disk.read(next_block, (uint8_t*)block_content);
      cout << "read data " << endl;
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
   /* std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";
    int dir_entry_index = -1;
    char name_array[56];

    for (int i = 0; i < ROOT_SIZE; i++)
    {
      if (dir_entries[i].first_blk != 0)
      {
        strcpy(name_array, sourcepath.c_str());

        if (dir_entries[i].file_name = sourcepath)
        {
          cout << "found file!";
          dir_entry_index = i;
          break;
        }
      }
    }

    if (dir_entry_index == -1)
    {
      cout << "No file with that name found";
      return 0;
    }
    return 0;*/
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";

    return 0;
}

// rm <filepath> removes / deletes the file <filepath>
int
FS::rm(std::string filepath)
{
    std::cout << "FS::rm(" << filepath << ")\n";
    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int
FS::append(std::string filepath1, std::string filepath2)
{
    std::cout << "FS::append(" << filepath1 << "," << filepath2 << ")\n";
    return 0;
}

// mkdir <dirpath> creates a new sub-directory with the name <dirpath>
// in the current directory
int
FS::mkdir(std::string dirpath)
{
    std::cout << "FS::mkdir(" << dirpath << ")\n";
    return 0;
}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int
FS::cd(std::string dirpath)
{
    std::cout << "FS::cd(" << dirpath << ")\n";
    return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int
FS::pwd()
{
    std::cout << "FS::pwd()\n";
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
