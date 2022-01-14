#include <iostream>
#include "fs.h"
#include <cstring>
#include <vector>
#include <typeinfo> // for debugging
using namespace std;

struct dir_entry dir_entries[ROOT_SIZE];
vector<int> dir_path;
int curr_dir_content[ROOT_SIZE];
 // Array of dir_entries indexes mapping our folder content to the dir_entries array.

int FS::file_fit_check(int num_blocks) //KLAAAAAAAAAAAAAAAAAAAAAAR
{
  int found_blocks = 0;
  for (int i = 0; i < 64; i++)
  {
    if (fat[i] == 0)
    {
      found_blocks++;
    }

    if (found_blocks == num_blocks)
    {
      return 1;
    }
  }
  return 0;
}

std::vector<int> FS::init_dir_content(std::vector<int> path) // "KLAAAAAAAAAAAAAAAAAAAAAAR"
{
   // return new dir content array so it can be assigned to temp places as well as global.
  // cout << "DID I CRASH HERE?????\n" << endl;
   // Is root folder
  cout << "3.1\n";
  if(path.size() == 0){

    /*
    Step 1: Find all folders in dir_entries
    Step 2: All items that are not part of another folders content must be in root director
    Step 3: Set orphan entries to current folder
    */
  // Get all the folders
   int folder_indexes[ROOT_SIZE];
   int dir_not_in_root[ROOT_SIZE];
   int dir_in_root[ROOT_SIZE];
   cout << "3.2\n";
   for (int i = 0; i < ROOT_SIZE; i++)
   {
     folder_indexes[i] = -1;
     dir_in_root[i] = -1;
     dir_not_in_root[i] = -1;
   }
   cout << "3.3\n";
   int folder_index_counter = 0;
   for(int i = 0; i < ROOT_SIZE; i++)
   {
      if(dir_entries[i].type == TYPE_DIR)
      {
        cout << "3.4\n";
        folder_indexes[folder_index_counter] = i;
        folder_index_counter++;
      }
   }
  //  cout << "folder_index_counter: " << folder_index_counter << "\n";


  //  //Nytt
  int dir_not_in_root_counter = 0;
  for (int i = 0; i < folder_index_counter; i++)
   {
    //  cout << folder_indexes[i] << "\n";
     int* dir_content = (int*)calloc(BLOCK_SIZE, sizeof(char));

     disk.read(dir_entries[folder_indexes[i]].first_blk, (uint8_t*)dir_content);
    //  if (dir_content[0] == -2)
    //  {
     //
    //  }

     for (int j = 0; j < ROOT_SIZE; j++)
     {
       if (dir_content[j] != -1)
       {
        //  cout << "dir_content: " << dir_content[j] << "\n";
         dir_not_in_root[dir_not_in_root_counter] = dir_content[j];
        //  cout << "dir_content: " << dir_content[j] << "\n";
         dir_not_in_root_counter++;
       }
     }
   }
   int dir_in_root_counter = 0;

   for (int i = 0; i < ROOT_SIZE; i++)
   {
    //  cout << "current i: " << i << "\n";

     if (dir_entries[i].first_blk == 0)
     {
       continue;
     }

     int i_found = 0;
     for (int j = 0; j < ROOT_SIZE; j++)
     {
      //  cout << "dir_not_in_root: " << dir_not_in_root[j] << "\n";
       if (i == dir_not_in_root[j])
       {
         i_found = 1;
         break;
       }
     }

     if (i_found == 0)
     {
      //  cout << "i not found: " << i << "\n";
       dir_in_root[dir_in_root_counter] = i;
       dir_in_root_counter ++;
     }
   }

   std::vector<int> return_value;
   // Save to memory
   for(int i = 0; i < ROOT_SIZE; i++)
   {
     return_value.push_back(dir_in_root[i]);
   }

   for(int i = 0; i < ROOT_SIZE/8; i++)
   {
    //  cout << return_value[i] << endl;
   }

   return return_value;
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

    // curr_dir_content[0] = -2;

    // Init root dir content
    // for(int i = 1; i < ROOT_SIZE; i++) {
    //   curr_dir_content[i] = -1;
    // }

        // Set the root folder
    // int* ptr = (int*)calloc(ROOT_SIZE, sizeof(int));

    // cout << "-- Starting init of root dir \n";
    // int root_dir_content[ROOT_SIZE];
    //
    // for (int i = 0; i < ROOT_SIZE; i++)
    // {
    //   root_dir_content[i] = -1;
    // }

    // int* ptr = (int*)calloc(ROOT_SIZE, sizeof(int));
    std::vector<int> root_dir_content;

    root_dir_content = init_dir_content(dir_path);

    for(int i = 0; i < ROOT_SIZE; i++)
    {
       curr_dir_content[i] = root_dir_content[i];
    }

}

FS::~FS()
{

}

// formats the disk, i.e., creates an empty file system
int
FS::format() //KLAAAAR
{
    //"*"*"*"*"*"*"*"*"*"*"*"FIXA PATHEN VID FORMAT*"*"*"*"*"*"*"""""""*"""*
    string empty_str = "";

    for (int i = 0; dir_path.size() != 0; i++)
    {
      dir_path.resize(dir_path.size() - 1);
    }

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

    for(int i = 0; i < ROOT_SIZE; i++)
    {
      curr_dir_content[i] = -1;
    }

    fat[ROOT_BLOCK] = FAT_EOF;
    disk.write(ROOT_BLOCK, (uint8_t*)dir_entries); // dir_entries in the file

    fat[FAT_BLOCK] = FAT_EOF;
    disk.write(FAT_BLOCK, (uint8_t*)fat); // Fat in the file
    std::cout << "FS::format()\n";
    return 0;
}

// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int
FS::create(std::string filepath) //KLAAAAAAAAAAAAAAAAAAAAAAAAAAR
{
    //get file size
    int size_of_file;
    string string_to_eval;
    string string_to_eval_temp;
    int start_block;

    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if (dir_entries[curr_dir_content[i]].file_name == filepath)
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

    cout << "size of file: " << size_of_file << "\n";

    //insert data into directory entry

    // check amount of block
    int num_blocks = size_of_file / BLOCK_SIZE + 1;
    int num_blocks_temp = size_of_file / BLOCK_SIZE;

    if (size_of_file==BLOCK_SIZE*num_blocks_temp)
    {
      num_blocks = num_blocks_temp;
    }

    int fitcheck = file_fit_check(num_blocks);

    if (fitcheck == 0)
    {
      cout << "File too large!\n";
      return 0;
    }

    int free_spaces[num_blocks];
    int free_space_counter = 0;
    for (int i = 0; i < BLOCK_SIZE/2; i++)
    {
      // check where the file can fit in the fat
      if (fat[i] == FAT_FREE)
      {
        free_spaces[free_space_counter] = i;
        free_space_counter++;
      }

      if (free_space_counter >= num_blocks)
      {
        // go back and fill
        start_block = 0;

        //set fat values
        int elements_in_fat_counter = 1;
        for(int j = start_block; j < 64; j++)
        {
          if (fat[j] == FAT_FREE)
          {
            if (start_block==0)
            {
              start_block = j;
            }

            if (elements_in_fat_counter == free_space_counter)
            {
              fat[j] = FAT_EOF;
              break;
            }

            fat[j] = free_spaces[elements_in_fat_counter];
            elements_in_fat_counter++;
          }
        }
        disk.write(FAT_BLOCK, (uint8_t*)fat);

        // Write to the disk
        for(int j = start_block; j < start_block + num_blocks; j++)
        {
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
    cout << "1\n";
    struct dir_entry temp_entry;

    // int* ptr = (int*)calloc(ROOT_SIZE, sizeof(int));
    // ptr = init_dir_content(dir_path);

    // for(int i = 0; i < ROOT_SIZE; i++)
    // {
    //   curr_dir_content[i] = ptr[i];
    // }

    // cout << "1\n";
    strcpy(temp_entry.file_name, filepath.c_str());

    temp_entry.size = size_of_file;
    cout << "dir_entry_size: " << temp_entry.size << " == func_size: " << size_of_file << "\n";
    temp_entry.first_blk = start_block;
    temp_entry.type = TYPE_FILE;
    temp_entry.access_rights = READ;
        cout << "2\n";
    // dir_entry to root
    int dir_entry_index;
    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if(dir_entries[i].first_blk == 0)
      {
        dir_entries[i] = temp_entry;
        dir_entry_index = i;
        break;
      }
    }
        cout << "3\n";


    for( int i = 0; i < ROOT_SIZE; i++)
    {
      if (curr_dir_content[i] == -1)
      {
        curr_dir_content[i] = dir_entry_index;
        break;
      }
    }
        cout << "4\n";

    disk.write(ROOT_BLOCK, (uint8_t*)dir_entries);


    if(dir_path.size() == 0)
    {
      return 0;
    }
    cout << "5\n";
    cout << dir_path.size() << endl;

    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT
    /**DONT TOUCH**/ cout << endl; //DONT TOUCH
    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT

    if (dir_path.size() != 0) // ÄNDRADE FRÅN 1 TILL 0, ABC FÖLJDE MED UPP TILL ROOT
    {
      // int curr_dir_id;
      // int* parent_dir_content = (int*)calloc(ROOT_SIZE, sizeof(int));

      // int parent_dir_content[ROOT_SIZE];
      // for (int i = 0; i < ROOT_SIZE; i++)
      // {
      //   parent_dir_content[i] = -1;
      // }

      // disk.read(dir_entries[parent_id].first_blk, (uint8_t*)parent_dir_content);

      // cout << "shit i gotz here ";
      // for(int i = 0; i < ROOT_SIZE; i++){
      //
      //   if(parent_dir_content[i] == dir_path.back())
      //   {
      //     // cout << "i gotz here \n";
      //     curr_dir_id = parent_dir_content[i];
      //     break;
      //   }
      // }
      // cout <<  "dir_block: " << dir_entries[curr_dir_id].first_blk << "\n";
      // int temp_dir_content[ROOT_SIZE];
      // for(int i = 0; i < ROOT_SIZE; i++)
      // {
      //   temp_dir_content[i] = -1;
      // }
      // std::vector<int> temp_dir_path = dir_path;
      // temp_dir_path.resize(temp_dir_path.size() -1);
      // disk.read(dir_entries[temp_dir_path.back()].first_blk, (uint8_t*)temp_dir_content);

      // for(int i = 0; i < ROOT_SIZE; i++) {
      //   cout << "curr_dir_id: "<< dir_entries[dir_path.back()].first_blk << "\n";
      // }
      int block_to_write = dir_entries[dir_path.back()].first_blk;
      disk.write(block_to_write, (uint8_t*)curr_dir_content);

    }

    // save_curr_dir();
    std::cout << "FS::create(" << filepath << ")\n";
    return 0;
}


// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath) ////KLAAAAAR
{
    std::cout << "FS::cat(" << filepath << ")\n";
    int blocks_to_read;
    int entry_index;
    int file_found = 0;

    // Find file
    for(int i = 0; i < ROOT_SIZE; i++){

      if(dir_entries[i].file_name == filepath)
      {
        file_found = 1;
        entry_index = i;
        blocks_to_read = dir_entries[i].size/BLOCK_SIZE;
        // Add rest block if exists
        if(dir_entries[i].size%BLOCK_SIZE > 0)
        {
          blocks_to_read++;
        }
        break;
      }
    }
    // if type = file
    if(dir_entries[entry_index].type == TYPE_FILE){
      if (file_found == 0)
      {
        cout << "No file with that name found, try again\n";
        return 0;
      }

      if(blocks_to_read == 0){
        return -1;
      }

      // read from disk
      char* block_content;
      block_content = (char*)calloc(BLOCK_SIZE, sizeof(char));
      char* read_data;
      read_data = (char*) calloc(blocks_to_read, BLOCK_SIZE);
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
      free(block_content);
      free(read_data);
      return 0;
    } else {
      
      cout << "Cannot not cat directory" << endl;
      // debug code below
      // int temp_dir_content[64];
      // disk.read(dir_entries[entry_index].first_blk, (uint8_t*)temp_dir_content);
      // for(int i = 0; i < ROOT_SIZE; i++){
      //   cout << temp_dir_content[i] << endl;
      // }
      return -1;
    }


}

// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls() ////KLAR FÖR ROOTEN MEN INTE FÖR DJUP
{
  // int dir_size = 0;
  // for (int i = 0; i < 64; i++)
  // {
  //     if (curr_dir_content[i] != -1)
  //     {
  //       dir_size++;
  //       cout << "dir size: " << dir_size << "\n";
  //     }
  // }
  // dir_size++

  for (int i = 0; i < sizeof(curr_dir_content)/sizeof(curr_dir_content[0]); i++)
      {
        if (curr_dir_content[i] != -1)
        {
          cout << dir_entries[curr_dir_content[i]].file_name << "                     " << dir_entries[curr_dir_content[i]].size << "\n";
        }
      }

  return 0;
}

// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int
FS::cp(std::string sourcepath, std::string destpath) ////KLAAAAAAAAAAAAAAAAAAAAAAAAAR
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

    // check if source is directory
      if(dir_entries[dir_entry_index].type == TYPE_DIR) {
        cout << "Cannot copy a directory." << endl;
        return -1;
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

    int blocks_to_read = dir_entries[dir_entry_index].size / BLOCK_SIZE + 1;
    int blocks_to_read_temp = dir_entries[dir_entry_index].size / BLOCK_SIZE;

    if (dir_entries[dir_entry_index].size==BLOCK_SIZE*blocks_to_read_temp)
    {
      blocks_to_read_temp = blocks_to_read_temp;
    }

    int fitcheck = file_fit_check(blocks_to_read);

    if (fitcheck == 0)
    {
      cout << "File too large!\n";
      return 0;
    }

    //GET FILE information

    uint32_t size_of_file = dir_entries[dir_entry_index].size;
    uint16_t first_block = dir_entries[dir_entry_index].first_blk;
    uint8_t type = dir_entries[dir_entry_index].type;
    uint8_t access_rights = dir_entries[dir_entry_index].access_rights;

    // cout << "1 \n";

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
      strcat(read_data, block_content);

      if(next_block != -1)
      {
        next_block = fat[next_block];
      }
    }
    // cout << "2 \n";
    //**''*''*'''*''*''**CREATE FILE copied from CREATE function**''*''*''*

    //get file size
    int start_block;
    string string_to_eval = read_data;
    // free(read_data);
    // convert string_to_eval to uint8_t*
    uint8_t* block = (uint8_t*)string_to_eval.c_str();

    size_of_file = string_to_eval.length();
    int size_of_file_temp = size_of_file;

    //FEEEEEEEEEL end här

    //insert data into directory entry
    cout << "3 \n";
    // check amount of block

        int free_spaces[blocks_to_read];
        int free_space_counter = 0;
        for (int i = 0; i < BLOCK_SIZE/2; i++)
        {
          // check where the file can fit in the fat
          if (fat[i] == FAT_FREE)
          {
            free_spaces[free_space_counter] = i;
            free_space_counter++;
          }

          if (free_space_counter >= blocks_to_read)
          {
            // go back and fill
            start_block = 0;

            //set fat values
            int elements_in_fat_counter = 1;
            for(int j = start_block; j < 64; j++)
            {
              if (fat[j] == FAT_FREE)
              {
                if (start_block==0)
                {
                  start_block = j;
                }

                if (elements_in_fat_counter == free_space_counter)
                {
                  fat[j] = FAT_EOF;
                  break;
                }

                fat[j] = free_spaces[elements_in_fat_counter];
                elements_in_fat_counter++;
              }
            }
        disk.write(FAT_BLOCK, (uint8_t*)fat); // Fat in the file

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
    cout << "4 \n";
    strcpy(temp_entry.file_name, destpath.c_str());

    temp_entry.size = size_of_file;
    temp_entry.first_blk = start_block;
    temp_entry.type = type;
    temp_entry.access_rights = access_rights;

    // vector<string> path = dir_path;
    // path.push_back(temp_entry.file_name);
    //
    // init_dir_content(path);

    // entry to root
    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if(dir_entries[i].first_blk == 0)
      {
        dir_entries[i] = temp_entry;
        dir_entry_index = i;
        break;
      }
    }

    int i;

    if (dir_path.size() == 1)
    {
      i = 0;
    }

    else
    {
      i = 1;
    }

    for(i; i < ROOT_SIZE; i++)
    {
      if (curr_dir_content[i] == -1)
      {
        // cout << "curr_dir_content[i]: " << curr_dir_content[i] << "\n";
        // cout << "dir_entry_index " << dir_entry_index << "\n";
        // cout << "current i:  " << dir_entry_index << "\n";

        curr_dir_content[i] = dir_entry_index;
        break;
      }
    }
    cout << "5 \n";
    disk.write(ROOT_BLOCK, (uint8_t*)dir_entries); // dir_entries in the file

    // int parent_id = curr_dir_content[0];

    // if (parent_id == -2)
    // {
    //   // parent_id = get_parent_index(dir_path);
    // }

    // else
    // {
    //   parent_id = curr_dir_content[0];
    // }

    if(dir_path.size() == 0)
    {
      return 0;
    }



    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT
    /**DONT TOUCH**/ cout << endl; //DONT TOUCH
    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT


    // 1. vill hitta curr_dir_id
    // 2. Läs hämta parents dir_content
    // 3.
    cout << "pathsize: " << dir_path.size();


    if (dir_path.size() != 1)
    {
      // int curr_dir_id;
      // int* parent_dir_content = (int*)calloc(ROOT_SIZE, sizeof(int));

      // int parent_dir_content[ROOT_SIZE];
      // for (int i = 0; i < ROOT_SIZE; i++)
      // {
      //   parent_dir_content[i] = -1;
      // }

      // disk.read(dir_entries[parent_id].first_blk, (uint8_t*)parent_dir_content);

      // cout << "shit i gotz here ";
      // for(int i = 0; i < ROOT_SIZE; i++){
      //
      //   if(parent_dir_content[i] == dir_path.back())
      //   {
      //     // cout << "i gotz here \n";
      //     curr_dir_id = parent_dir_content[i];
      //     break;
      //   }
      // }
      // cout <<  "dir_block: " << dir_entries[curr_dir_id].first_blk << "\n";
      // int temp_dir_content[ROOT_SIZE];
      // for(int i = 0; i < ROOT_SIZE; i++)
      // {
      //   temp_dir_content[i] = -1;
      // }
      // std::vector<int> temp_dir_path = dir_path;
      // temp_dir_path.resize(temp_dir_path.size() -1);
      // disk.read(dir_entries[temp_dir_path.back()].first_blk, (uint8_t*)temp_dir_content);

      // for(int i = 0; i < ROOT_SIZE; i++) {
      //   cout << "curr_dir_id: "<< dir_entries[dir_path.back()].first_blk << "\n";
      // }
      int block_to_write = dir_entries[dir_path.back()].first_blk;
      disk.write(block_to_write, (uint8_t*)curr_dir_content);

    }

    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destpath) //KLAAAAAAAAAAAAAAAAAAAAAAR
{
    std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";


    if (destpath == "..")
    {
      //kvar att fixa
      if (dir_path.size() == 0)
      {
        cout << "Already in root!\n";
        return 0;
      }

      if(dir_path.size() -1 == 0) 
      {
        // make to orphan
        // find index
        int dir_entry_index;
        for(int i = 0; ROOT_SIZE; i++) 
        {
          if(dir_entries[curr_dir_content[i]].file_name == sourcepath)
          {
            dir_entry_index = curr_dir_content[i];
            break;
          }
        }

        for(int i = 0; i < ROOT_SIZE; i++ ) {
          if(curr_dir_content[i] == dir_entry_index) 
          {
            curr_dir_content[i] = -1;
            break;
          }
        }
        disk.write(dir_entries[dir_path.back()].first_blk, (uint8_t*)curr_dir_content);
        return 0;
      }

      vector<int> dir_path_temp = dir_path;
      dir_path_temp.resize(dir_path_temp.size()-1);
      // General case
      int dir_entry_index;
      for(int i = 0; ROOT_SIZE; i++) 
      {
        if(dir_entries[curr_dir_content[i]].file_name == sourcepath)
        {
          dir_entry_index = curr_dir_content[i];
          break;
        }
      }

      for(int i = 0; i < ROOT_SIZE; i++ )
      {
        if(curr_dir_content[i] == dir_entry_index) 
        {
          curr_dir_content[i] = -1;
          break;
        }
      }
      int parent_dir_content[BLOCK_SIZE];
      disk.read(dir_entries[dir_path_temp.back()].first_blk, (uint8_t*)parent_dir_content);

      for(int i = 0; i < ROOT_SIZE; i++) 
      {
        if(parent_dir_content[i] == -1)
        {
          parent_dir_content[i] = dir_entry_index;
          break;
        }
      }

      disk.write(dir_entries[dir_path_temp.back()].first_blk, (uint8_t*)parent_dir_content);
      
      return 0;
    }


    //find index of sourcefile

    int dir_entry_index;
    for (int i = 0; i < ROOT_SIZE; i++)
    {
      if (dir_entries[curr_dir_content[i]].file_name == sourcepath)
      {
        dir_entry_index = curr_dir_content[i];
        break;
      }
    }

    // Is destpath a directory?
    for(int i = 0; i < ROOT_SIZE; i++)
    {
      
      if(dir_entries[curr_dir_content[i]].file_name == destpath && dir_entries[curr_dir_content[i]].type == TYPE_DIR)
      {
        //**""**""**""**""  enter folder  **""**""**""**""
        int dest_dir_index = curr_dir_content[i];

        //Find folder
        vector<int> dir_path_temp = dir_path;

        //*""**""**""**""**""GÖR OM DESTPATH TILL INDEX**""**""*""*""*"*"*"*"*
        dir_path_temp.push_back(dest_dir_index);
        int dir_entry_index;

        for(int i = 0; ROOT_SIZE; i++) 
        {
          if(dir_entries[curr_dir_content[i]].file_name == sourcepath)
          {
            dir_entry_index = curr_dir_content[i];
            break;
          }
        }

        for(int i = 0; i < ROOT_SIZE; i++)
        {
          if(curr_dir_content[i] == dir_entry_index)
          {
            curr_dir_content[i] = -1;
          }
        }

        if(dir_path.size() == 0)
        {
          int new_dir_content[BLOCK_SIZE];
          cout << "gonna riid \n";
          disk.read(dir_entries[dir_path_temp.back()].first_blk, (uint8_t*)new_dir_content);

          cout << "starting loop 3" << endl;
          for(int i = 0; i < ROOT_SIZE; i++)
          {
            if(new_dir_content[i] == -1)
            {
              new_dir_content[i] = dir_entry_index;
              break;
            }
          }
          disk.write(dir_entries[dir_path_temp.back()].first_blk, (uint8_t*)new_dir_content);
          
          return 0;
        }


        cout << "gonna wrajt \n";
        cout << dir_path.back() << endl;

        cout << dir_entries[dir_path.back()].first_blk << endl;

        disk.write(dir_entries[dir_path.back()].first_blk, (uint8_t*)curr_dir_content);

        int new_dir_content[BLOCK_SIZE];
        cout << "gonna riid \n";
        disk.read(dir_entries[dir_path_temp.back()].first_blk, (uint8_t*)new_dir_content);

        cout << "starting loop 3" << endl;
        for(int i = 0; i < ROOT_SIZE; i++)
        {
          if(new_dir_content[i] == -1)
          {
            new_dir_content[i] = dir_entry_index;
            break;
          }
        }
        disk.write(dir_entries[dir_path_temp.back()].first_blk, (uint8_t*)new_dir_content);
        
        return 0;
      }
   

    }

    // find source file
    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if(dir_entries[i].file_name == sourcepath)
      {
        strcpy(dir_entries[i].file_name, destpath.c_str());
        disk.write(ROOT_BLOCK, (uint8_t*)dir_entries); // dir_entries in the file

        return 0;
      }
    }

    cout << "No file with the name: " << sourcepath.c_str() << "\n";
    return 0;
 }

// rm <filepath> removes / deletes the file <filepath>
int
FS::rm(std::string filepath) //KLAAAAAAAAAAAAAAAAAAAAAAAAAR
{
    std::cout << "FS::rm(" << filepath << ")\n";

    int name_found = 0;
    int dir_entry_index;
    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if(dir_entries[curr_dir_content[i]].file_name == filepath)
      {
        name_found = 1;
        dir_entry_index = curr_dir_content[i];

        // if folder with content, throw error
        if(dir_entries[dir_entry_index].type == TYPE_DIR)
        {
          int temp_dir_content[BLOCK_SIZE];

          disk.read(dir_entries[dir_entry_index].first_blk, (uint8_t*)temp_dir_content);
          for(int j = 0; j < ROOT_SIZE; j++)
          {
            cout << "temp_dir_content " << temp_dir_content[j] << endl;
            if (temp_dir_content[j] != -1)
            {
              cout << "Empty the directory before removing it." << endl;
              return -1;
              
            }
          }
        }


        // Clear fat
        int blocks_to_read = dir_entries[curr_dir_content[i]].size/BLOCK_SIZE;
        if(dir_entries[curr_dir_content[i]].size%BLOCK_SIZE > 0)
        {
          blocks_to_read++;
        }

        int curr_block = dir_entries[curr_dir_content[i]].first_blk;
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
        dir_entries[curr_dir_content[i]] = temp_entry;

        for (int j = 0; j < ROOT_SIZE; j++)
        {
          if (curr_dir_content[j] == dir_entry_index)
          {
            curr_dir_content[j] = -1;
          }
        }
      }
    }

    if (name_found == 0)
    {
      cout << "No file with that name found, try again\n";
      return 0;
    }

    for (int j = 0; j < ROOT_SIZE; j++)
    {

      cout << "curr_dir_content: " << curr_dir_content[j] << "\n";
    }

    // Save curr dir
    int curr_dir_id;

    if(dir_path.size() == 0)
    {
      disk.write(FAT_BLOCK, (uint8_t*)fat);
      disk.write(ROOT_BLOCK, (uint8_t*)dir_entries); 
      return 0;
    }
    
    // if remove folder, shall we remove content? NO!!!
    int parent_dir_content[BLOCK_SIZE];
    disk.read(dir_entries[dir_path.back()].first_blk, (uint8_t*)parent_dir_content);

    for (int i = 0; i < ROOT_SIZE; i++)
    {
      if(parent_dir_content[i] == dir_entry_index)
      {
        parent_dir_content[i] = -1;
        curr_dir_content[i] = -1; //stoopid
        break;
      }
    }

    disk.write(dir_entries[dir_path.back()].first_blk, (uint8_t*)parent_dir_content);

    // cout <<  "dir_block: " << dir_entries[parent_id].first_blk << "\n";
    // int* parent_dir_content = (int*)calloc(BLOCK_SIZE, sizeof(char));
    // cout << "imma read \n";
    // disk.read(dir_entries[parent_id].first_blk, (uint8_t*)parent_dir_content);
    // cout << "shit i gotz here ";
    // for(int i = 0; i < ROOT_SIZE; i++){

    //   if(parent_dir_content[i] == dir_path.back())
    //   {
    //     curr_dir_id = parent_dir_content[i];
    //   }
    // }
    // cout <<  "dir_block: " << dir_entries[curr_dir_id].first_blk << "\n";
    // disk.write(dir_entries[curr_dir_id].first_blk, (uint8_t*)curr_dir_content);

    // std::cout << "FS::create(" << filepath << ")\n";


    disk.write(ROOT_BLOCK, (uint8_t*)dir_entries); // dir_entries in the file
    disk.write(FAT_BLOCK, (uint8_t*)fat); // Fat in the file

    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int
FS::append(std::string filepath1, std::string filepath2) //KLAAAAAAAAAAAAAAAAAAAAAAAAAAAR
{
    std::cout << "FS::append(" << filepath1 << "," << filepath2 << ")\n";

    // *""*""*""*""*"" FIND FILES *""*""*""*""*""**
    //File 1
    int dir_entry_index_1 = -1;
    int blocks_to_read_1;
    int entry_index_1;
    int file_found_1 = 0;

    // Find file
    for(int i = 0; i < ROOT_SIZE; i++){

      if(dir_entries[curr_dir_content[i]].file_name == filepath1)
      {
        file_found_1 = 1;
        entry_index_1 = i;
        // check if dir
        if(dir_entries[entry_index_1].type == TYPE_DIR)
        {
          cout << "Cannot not append directory." << endl;
          return -1;
        }

        blocks_to_read_1 = dir_entries[curr_dir_content[i]].size/BLOCK_SIZE;
        dir_entry_index_1 = curr_dir_content[i];
        // Add rest block if exists
        if(dir_entries[curr_dir_content[i]].size%BLOCK_SIZE > 0)
        {
          blocks_to_read_1++;
        }
        break;
      }
    }


    if (file_found_1 == 0)
    {
      cout << "No file with that name found, try again\n";
      return 0;
    }

    if(blocks_to_read_1 == 0){
      return -1;
    }

    //File 2
    int dir_entry_index_2 = -1;
    int blocks_to_read_2;
    int entry_index_2;
    int file_found_2 = 0;

    // Find file
    for(int i = 0; i < ROOT_SIZE; i++){

      if(dir_entries[curr_dir_content[i]].file_name == filepath2)
      {
        file_found_2 = 1;
        //kanske behöver fixa entry_index
        entry_index_2 = i;
        // check if dir
        if(dir_entries[entry_index_2].type == TYPE_DIR)
        {
          cout << "Cannot not append directory." << endl;
          return -1;
        }
        blocks_to_read_2 = dir_entries[curr_dir_content[i]].size/BLOCK_SIZE;
        dir_entry_index_2 = curr_dir_content[i];
        // Add rest block if exists
        if(dir_entries[curr_dir_content[i]].size%BLOCK_SIZE > 0)
        {
          blocks_to_read_2++;
        }
        break;
      }
    }

    if (file_found_2 == 0)
    {
      cout << "No file with that name found, try again\n";
      return 0;
    }

    if(blocks_to_read_2 == 0){
      return -1;
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
    char* block_content_1;
    block_content_1 = (char*)calloc(BLOCK_SIZE, sizeof(char));
    char* read_data_1;
    read_data_1 = (char*) calloc(blocks_to_read_1,BLOCK_SIZE*2);
    memset(read_data_1, 0, BLOCK_SIZE*blocks_to_read_1);
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

    //file 2;
    char* block_content_2;
    block_content_2 = (char*)calloc(BLOCK_SIZE, sizeof(char));
    char* read_data_2;
    read_data_2 = (char*) calloc(blocks_to_read_2+blocks_to_read_1,BLOCK_SIZE);
    memset(read_data_2, 0, BLOCK_SIZE*(blocks_to_read_2+blocks_to_read_1));
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
    int num_blocks_temp = size_of_file / BLOCK_SIZE;

    if (size_of_file==BLOCK_SIZE*num_blocks_temp)
    {
      num_blocks = num_blocks_temp;
    }

    int fitcheck = file_fit_check(num_blocks);

    if (fitcheck == 0)
    {
      cout << "File too large!\n";
      return 0;
    }

    // *""*""*""*""*"" REMOVE FILE 2 *""*""*""*""*""**
    rm(filepath2);

    int free_spaces[num_blocks];
    int free_space_counter = 0;
    for (int i = 0; i < BLOCK_SIZE/2; i++)
    {
      // check where the file can fit in the fat
      if (fat[i] == FAT_FREE)
      {
        free_spaces[free_space_counter] = i;
        free_space_counter++;
      }

      if (free_space_counter >= num_blocks)
      {
        // go back and fill
        start_block = 0;

        //set fat values
        int elements_in_fat_counter = 1;
        for(int j = start_block; j < 64; j++)
        {
          if (fat[j] == FAT_FREE)
          {
            if (start_block==0)
            {
              start_block = j;
            }

            if (elements_in_fat_counter == free_space_counter)
            {
              fat[j] = FAT_EOF;
              break;
            }

            fat[j] = free_spaces[elements_in_fat_counter];
            elements_in_fat_counter++;
          }
        }
        disk.write(FAT_BLOCK, (uint8_t*)fat);

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

    strcpy(temp_entry.file_name, filepath2.c_str());

    temp_entry.size = size_of_file;
    temp_entry.first_blk = start_block;
    temp_entry.type = type_2;
    temp_entry.access_rights = access_rights_2;
    // dir_entry to root
    int dir_entry_index;
    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if(dir_entries[i].first_blk == 0)
      {
        dir_entries[i] = temp_entry;
        dir_entry_index = i;
        break;
      }
    }
        cout << "3\n";


    for( int i = 0; i < ROOT_SIZE; i++)
    {
      if (curr_dir_content[i] == -1)
      {
        curr_dir_content[i] = dir_entry_index;
        break;
      }
    }
        cout << "4\n";

    disk.write(ROOT_BLOCK, (uint8_t*)dir_entries);


    if(dir_path.size() == 0)
    {
      return 0;
    }
    cout << "5\n";

    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT
    /**DONT TOUCH**/ cout << endl; //DONT TOUCH
    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT

    if (dir_path.size() != 1)
    {
      // int curr_dir_id;
      // int* parent_dir_content = (int*)calloc(ROOT_SIZE, sizeof(int));

      // int parent_dir_content[ROOT_SIZE];
      // for (int i = 0; i < ROOT_SIZE; i++)
      // {
      //   parent_dir_content[i] = -1;
      // }

      // disk.read(dir_entries[parent_id].first_blk, (uint8_t*)parent_dir_content);

      // cout << "shit i gotz here ";
      // for(int i = 0; i < ROOT_SIZE; i++){
      //
      //   if(parent_dir_content[i] == dir_path.back())
      //   {
      //     // cout << "i gotz here \n";
      //     curr_dir_id = parent_dir_content[i];
      //     break;
      //   }
      // }
      // cout <<  "dir_block: " << dir_entries[curr_dir_id].first_blk << "\n";
      // int temp_dir_content[ROOT_SIZE];
      // for(int i = 0; i < ROOT_SIZE; i++)
      // {
      //   temp_dir_content[i] = -1;
      // }
      // std::vector<int> temp_dir_path = dir_path;
      // temp_dir_path.resize(temp_dir_path.size() -1);
      // disk.read(dir_entries[temp_dir_path.back()].first_blk, (uint8_t*)temp_dir_content);

      // for(int i = 0; i < ROOT_SIZE; i++) {
      //   cout << "curr_dir_id: "<< dir_entries[dir_path.back()].first_blk << "\n";
      // }
      int block_to_write = dir_entries[dir_path.back()].first_blk;
      disk.write(block_to_write, (uint8_t*)curr_dir_content);

    }

    // save_curr_dir();
    return 0;
}

int
FS::mkdir(std::string dirpath) //KLAaaaaaaaaaaaaaaaar
{
    std::cout << "FS::mkdir(" << dirpath << ")\n";
    int start_block;
    int num_blocks = 1;

    int fitcheck = file_fit_check(num_blocks);

    if (fitcheck == 0)
    {
      cout << "Disk full!\n";
      return 0;
    }

    // check if name already exists in directory
    for (int i = 0; i < ROOT_SIZE; i++)
    {
      if(dir_entries[curr_dir_content[i]].file_name == dirpath)
      {
        cout << "Name already exists"<< endl;
        return -1;
      }
    }

    int free_spaces[num_blocks];
    int free_space_counter = 0;
    cout << "-2\n";
      for (int i = 0; i < BLOCK_SIZE/2; i++) {
        // check where the file can fit in the fat
        if (fat[i] == FAT_FREE) {
          free_spaces[free_space_counter] = i;
          free_space_counter++;
        }
        cout << "-1\n";
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
          disk.write(FAT_BLOCK, (uint8_t*)fat);

          // write dir entry list
          struct dir_entry temp_entry;
          cout << "0\n";
          // write content list to disk

          int temp_dir_content[ROOT_SIZE]; // TODO ADD PARENT INDEX FIRST

          for(int i = 0; i < ROOT_SIZE; i++)
          {
            temp_dir_content[i] = -1;
          }

          cout << "1\n";
          strcpy(temp_entry.file_name, dirpath.c_str());

          temp_entry.first_blk = start_block;
          temp_entry.type = TYPE_DIR;
          temp_entry.access_rights = READ;
          temp_entry.size = sizeof(temp_dir_content);

          // dir_entry to root block
          //find empty slot in dir entries
          int dir_entry_index;
          for(int r = 0; r < ROOT_SIZE; r++)
          {
            if(dir_entries[r].first_blk == 0)
            {
              // cout <<"Dir located at index " << r << endl;
              dir_entries[r] = temp_entry;
              dir_entry_index = r;
              break;
            }
          }
          cout << "2\n";

          int parent_index = dir_path.back();
          // cout << "parent index: " << parent_index << "\n";
          int parent_first_blk = dir_entries[parent_index].first_blk;

          if (dir_path.size() != 0)
          {
            for (int j = 0; j < ROOT_SIZE; j ++)
            {
              if (curr_dir_content[j] == -1)
              {
                cout << "curr_dir_content: " <<curr_dir_content[j] << endl;
                curr_dir_content[j] = dir_entry_index;
                break;
              }
            }
            cout << "3\n";
            disk.write(parent_first_blk, (uint8_t*)curr_dir_content);
          }
          cout << "4\n";
          disk.write(ROOT_BLOCK, (uint8_t*)dir_entries);
          disk.write(start_block,  (uint8_t*)temp_dir_content);
          // for (int q = 0; q < ROOT_SIZE; q++)
          // {
          //   cout << "temp dir content: " << temp_dir_content
          // }

          // cout << "path size: " << dir_path.size() << "\n";

          if (dir_path.size() == 0)
          {
            std::vector<int> root_dir_content;
            root_dir_content = init_dir_content(dir_path);

            for(int i = 0; i < ROOT_SIZE; i++)
            {
               curr_dir_content[i] = root_dir_content[i];
            }
          }
          cout << "5\n";

          break;
        }
      }
    return 0;

}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int
FS::cd(std::string new_dir) // KLAAAAAAAAAAAAAAAAAAAAAAR
{
    std::cout << "FS::cd(" << new_dir << ")\n";

    // cout << "1\n";
  if (new_dir == "..")
  {
  // if new directory is ".." then we go up one level.
    if(dir_path.size() == 0)
    {
      cout << "Can't go futher up, you're at root. \n";
      return -1;
    }

    if (dir_path.size() - 1 == 0)
    {
      cout << "1\n";
      std::vector<int> root_dir_content;
      cout << "dir path size before : " << dir_path.size() << "\n";
      dir_path.resize(dir_path.size() - 1);
      cout << "dir path size after : " << dir_path.size() << "\n";
      root_dir_content = init_dir_content(dir_path);

      cout << "size of root_dir_content is " << root_dir_content.size() << "\n";
      for(int i = 0; i < ROOT_SIZE; i++)
      {
        cout << "curr_dir_content: " << curr_dir_content[i] << "\n";
        cout << "root_dir_content[] = " << root_dir_content[i] << endl;
        curr_dir_content[i] = root_dir_content[i];
      }

        cout << "3\n";
      return 0;
    }

    dir_path.resize(dir_path.size() - 1);
    // Set new folder dir content

    int new_dir_content[BLOCK_SIZE];
    for (int i = 0; i < ROOT_SIZE; i++)
    {
      new_dir_content[i] = -1;
      curr_dir_content[i] = -1;
    }

    disk.read(dir_entries[dir_path.back()].first_blk, (uint8_t*)new_dir_content);

    for (int i = 0; i < ROOT_SIZE; i++)
    {
      curr_dir_content[i] = new_dir_content[i];
    }


    // get new dir content
    return 0;
  }

  else
  {
    // cout << "2\n";

    for (int i = 0; i < ROOT_SIZE; i++)
    {
      if (dir_entries[curr_dir_content[i]].file_name == new_dir && dir_entries[curr_dir_content[i]].type == TYPE_DIR)
      {
        dir_path.resize(dir_path.size() + 1);
        dir_path[dir_path.size() -1 ] = curr_dir_content[i]; // Not using 


        int new_dir_content[BLOCK_SIZE];
        for (int i = 0; i < ROOT_SIZE; i++)
        {
          new_dir_content[i] = -1;
          curr_dir_content[i] = -1;
        }

        disk.read(dir_entries[dir_path.back()].first_blk, (uint8_t*)new_dir_content);

        for(int i = 0; i < ROOT_SIZE; i++)
        {

          curr_dir_content[i] = new_dir_content[i];
        }
        cout << "exiting \n";
        return 0;
      }
    }
  }
    cout << "directory not found" << endl;
    return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int
FS::pwd() //KLAAAAAAAAAAAAAAAAAAAAAAR
{
    std::cout << "FS::pwd()\n";
    string output_str = "";
    string temp;

    if(dir_path.size() > 0 )
    {
      // cout << "1\n";

      for(int i = 0; i < dir_path.size(); i++)
      {
        temp = dir_entries[dir_path[i]].file_name;
        output_str.append("/" + temp);

      }


    }

    else
    {
      output_str.append("/");
    }


    cout << output_str << std::endl;
    return 0;
}

// chmod <accessrights> <filepath> changes the access rights for the
// file <filepath> to <accessrights>.
int
FS::chmod(std::string accessrights, std::string filepath) // Inte klar
{
    std::cout << "FS::chmod(" << accessrights << "," << filepath << ")\n";

    //DEBUGGING
    for (int i = 0; i < 64 ; i++){
      cout << "fat[" << i << "] = " << fat[i] << "\n";
    }

    for (int i = 0; i < 64 ; i++){
      if(dir_entries[i].first_blk < 0 || dir_entries[i].first_blk > ROOT_SIZE){
        continue;
      }
      cout << "dir_entry[" << i << "] = " << dir_entries[i].file_name << "   Size == "<< dir_entries[i].size << "\n";
    }

    cout << "current directory: \n";
    for (int i = 0; i < dir_path.size(); i++)
    {
      cout << "file " << i << ": " << dir_path[i] << "\n";
    }

    cout << "\n";
    
    for (int i = 0; i < ROOT_SIZE; i++)
    {
      cout << "file " << i << ": " << curr_dir_content[i] << "\n";
    } 


    //ACTUAL CODE
    //find file
    int file_index = -1;
    for (int i = 0; i < 64; i++)
    {
      if (dir_entries[curr_dir_content[i]].file_name == filepath)
      {
        file_index = curr_dir_content[i];
        break;
      }
    }

    if (file_index == -1)
    {
      cout << "File not found\n";
    }

    int accessrights_int = -1;

    if (accessrights == "---")
    {
      accessrights_int = 0;
    }

    else if (accessrights == "r--")
    {
      accessrights_int = READ;
    }

    else if (accessrights == "rw-")
    {
      accessrights_int = READ + WRITE;
    }

    else if (accessrights == "rwx")
    {
      accessrights_int = READ + WRITE + EXECUTE;
    }

    else if (accessrights == "r-x")
    {
      accessrights_int = READ + EXECUTE;
    }

    else if (accessrights == "-w-")
    {
      accessrights_int = WRITE;
    }

    else if (accessrights == "-wx")
    {
      accessrights_int = WRITE + EXECUTE;
    }

    else if (accessrights == "--x")
    {
      accessrights_int = EXECUTE;
    }

    else if (accessrights == "r-x")
    {
      accessrights_int = READ + EXECUTE;
    }

    else
    {
      cout << "Wrong format on input\n";
      return 0;
    }


    //Change file accessrights
    dir_entries[file_index].access_rights = accessrights_int;

    disk.write(ROOT_BLOCK, (uint8_t*)dir_entries);
    return 0;
}
