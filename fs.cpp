#include <iostream>
#include "fs.h"
#include <cstring>
#include <vector>
#include <typeinfo> // for debugging
#include <iomanip>
#include <istream>
#include <streambuf>
using namespace std;

struct dir_entry dir_entries[ROOT_SIZE];
vector<int> dir_path;
int curr_dir_content[ROOT_SIZE];
 // Array of dir_entries indexes mapping our folder content to the dir_entries array.

struct membuf : std::streambuf
{
    membuf(char* begin, char* end)
    {
        this->setg(begin, begin, end);
    }
};

//SKA KUNNA HANTERA .. i path A/../B
std::vector<int> FS::string_to_vector_converter(string destpath, int from_rm, int from_cd)
{
  std::vector<int> dir_path_temp = dir_path;
  std::vector<int> path;
  std::vector<string> path_str;
  string dir;
  char delimiter = '/';
  int temp_dir_content[BLOCK_SIZE];
  int destpath_size = destpath.length();
  char destpath_char[destpath_size];
  strcpy(destpath_char, destpath.c_str());

  membuf sbuf(destpath_char, destpath_char + sizeof(destpath_char));
  istream in(&sbuf);

  for (int i = 0; i < ROOT_SIZE; i++)
  {
    temp_dir_content[i] = curr_dir_content[i];
  }

  while (getline(in, dir, delimiter))
  {
    path_str.push_back(dir);
  }

  // for (int i = 0; i < path_str.size(); i++)
  // {
  //     cout << "path_string: "<< path_str[path_str.size() - i] <<"\n";
  // }
  if (path_str.size() == 1)
  {
    cout << "dir_path_temp.size(): " << dir_path_temp.size() << "\n";
    cout << "le relative path\n";
    return dir_path_temp;
  }

  int counter_stop = path_str.size();
  cout << "counter_stop= " << counter_stop << "\n";
  int counter = 0;
  int depth_counter = 0;
  string dir_to_find;
  for (int i = 0; i < counter_stop; i++)
  {
    cout << "path i is now: " << path_str[i] << endl;
    dir_to_find = path_str[i];
    cout << path_str[i] << "\n";


    if (dir_to_find == "..")
    {
      counter++;
      depth_counter++;
      if(depth_counter > dir_path.size())
      {
        cout << "depth_counter= " << depth_counter << "\n";
        std::vector<int> return_path;
        return_path.push_back(-1);
        return return_path;

      }
      cout << "in .. \n";
      dir_path_temp.resize(dir_path_temp.size() - 1);
      cout << "dir_path_temp.size(): " << dir_path_temp.size() << "\n";
      if (dir_path_temp.size() == 0)
      {
        std::vector<int> root_dir_content;
        root_dir_content = init_dir_content(dir_path_temp);
        for(int i = 0; i < ROOT_SIZE; i++)
        {
          cout << "root_dir_content[i]: " << root_dir_content[i] << "\n";
          temp_dir_content[i] = root_dir_content[i];
        }
        continue;
      }
      disk.read(dir_entries[dir_path_temp.back()].first_blk, (uint8_t*)temp_dir_content);
      if (counter == counter_stop)
      {
        for (int i = 0; i < dir_path_temp.size(); i++)
        {
            cout << "dirpath_temp: "<< dir_path_temp[i] <<"\n";
            cout << "dirpath_size: "<< dir_path_temp.size() <<"\n";
        }
        cout << "exiting\n";
        return dir_path_temp;
      }
      continue;
    }

    for (int j = 0; j < ROOT_SIZE; j++)
    {
      if (dir_entries[temp_dir_content[j]].file_name == dir_to_find && dir_entries[temp_dir_content[j]].type == TYPE_DIR)
      {
        //if folder then break
        if((dir_entries[temp_dir_content[j]].type == TYPE_DIR && counter_stop - counter == 1 ) && from_cd == 0)
        {
          cout << "Last item in path is folder. "  << endl;
          return dir_path_temp;
        }
        int dir_to_append = 0;
        cout << "8\n";
        dir_to_append = temp_dir_content[j];
        cout << dir_to_append << "\n";
        dir_path_temp.push_back(dir_to_append);
        disk.read(dir_entries[dir_to_append].first_blk, (uint8_t*)temp_dir_content);

        for (int i = 0; i < dir_path_temp.size(); i++)
        {
          cout << "dir_path_temp[i]: " << dir_path_temp[i] << "\n";
        }
        cout << "9\n";
        counter++;
        break;
      }
    }
  }

  if (from_rm == 1)
  {
    int dir_to_append = 0;
    for (int j = 0; j < ROOT_SIZE; j++)
    {
      if (dir_entries[temp_dir_content[j]].file_name == dir_to_find && dir_entries[temp_dir_content[j]].type == TYPE_FILE)
      {
        dir_to_append = temp_dir_content[j];
        dir_path_temp.push_back(dir_to_append);
        break;
      }
    }
    dir_path_temp.push_back(dir_to_append);
  }
  for(int i = 0; i < dir_path_temp.size(); i++)
  {
    cout << dir_path_temp[i] << endl;
  }
  cout << "exiting\n";
  return dir_path_temp;
}

int FS::accessrights_check(int dir_entry_index, int accessrights)
{

  int dest_file_accessrights = dir_entries[dir_entry_index].access_rights;

  //READ check
  if (accessrights == READ)
  {
    if (dest_file_accessrights == READ || dest_file_accessrights == READ + WRITE || dest_file_accessrights == READ + WRITE + EXECUTE || dest_file_accessrights == READ + EXECUTE)
    {
      return 0;
    }
    return -1;
  }
  //WRITE check
  if (accessrights == WRITE)
  {
    if (dest_file_accessrights == WRITE || dest_file_accessrights == READ + WRITE || dest_file_accessrights == READ + WRITE + EXECUTE || dest_file_accessrights == WRITE + EXECUTE)
    {
      return 0;
    }
    return -1;
  }
}

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
   // Is root folder
  // if(path.size() == 0)
  // {
      /*
      Step 1: Find all folders in dir_entries
      Step 2: All items that are not part of another folders content must be in root director
      Step 3: Set orphan entries to current folder
      */

    // Get all the folders
     int folder_indexes[ROOT_SIZE];
     int dir_not_in_root[ROOT_SIZE];
     int dir_in_root[ROOT_SIZE];

     for (int i = 0; i < ROOT_SIZE; i++)
     {
       folder_indexes[i] = -1;
       dir_in_root[i] = -1;
       dir_not_in_root[i] = -1;
     }

     int folder_index_counter = 0;
     for(int i = 0; i < ROOT_SIZE; i++)
     {
        if(dir_entries[i].type == TYPE_DIR)
        {
          folder_indexes[folder_index_counter] = i;
          folder_index_counter++;
        }
     }

    //  //Nytt
    int dir_not_in_root_counter = 0;
    for (int i = 0; i < folder_index_counter; i++)
     {
       int* dir_content = (int*)calloc(BLOCK_SIZE, sizeof(char));
       disk.read(dir_entries[folder_indexes[i]].first_blk, (uint8_t*)dir_content);

       for (int j = 0; j < ROOT_SIZE; j++)
       {
         if (dir_content[j] != -1)
         {
           dir_not_in_root[dir_not_in_root_counter] = dir_content[j];
           dir_not_in_root_counter++;
         }
       }
     }
     int dir_in_root_counter = 0;

     for (int i = 0; i < ROOT_SIZE; i++)
     {
       if (dir_entries[i].first_blk == 0)
       {
         continue;
       }

       int i_found = 0;
       for (int j = 0; j < ROOT_SIZE; j++)
       {
         if (i == dir_not_in_root[j])
         {
           i_found = 1;
           break;
         }
       }

       if (i_found == 0)
       {
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
     return return_value;
 // }
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
FS::create(std::string filepath) //KLAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAR
{
    //get file size
    int size_of_file;
    string string_to_eval;
    string string_to_eval_temp;
    int start_block;
    cout << "1\n";
    std::vector<int> dir_path_temp = string_to_vector_converter(filepath);
    if (dir_path_temp.size() >= 1)
    {
      if(dir_path_temp.back() == -1)
      {
        cout << "error cant go above root" << endl;
        return -1;
      }
    }
    std::vector<int> dir_path_temp_temp = dir_path;
    int temp_curr_dir_content[BLOCK_SIZE];

    for(int r = 0; r < ROOT_SIZE; r++)
    {
      temp_curr_dir_content[r] = curr_dir_content[r];
    }
    cout << "2\n";
    if (dir_path_temp.size() != dir_path.size() || dir_path_temp != dir_path)
    {
      cout << "2.2\n";
      dir_path_temp_temp = dir_path_temp;
      cout << dir_path_temp_temp.back() << "\n";
      disk.read(dir_entries[dir_path_temp_temp.back()].first_blk, (uint8_t*)temp_curr_dir_content);

      //Ful lösning för att hitta namnet
      std::vector<int> dir_path_name_find = dir_path;
      std::vector<int> path;
      std::vector<string> path_str;
      string dir;
      char delimiter = '/';
      int temp_dir_content[BLOCK_SIZE];
      int filepath_size = filepath.length();
      char filepath_char[filepath_size];
      strcpy(filepath_char, filepath.c_str());

      membuf sbuf(filepath_char, filepath_char + sizeof(filepath_char));
      istream in(&sbuf);

      while (getline(in, dir, delimiter))
      {
        path_str.push_back(dir);
      }
      filepath = path_str.back();
    }
    cout << "3\n";
    for(int i = 0; i < ROOT_SIZE; i++)
    {
      cout << "temp_curr_dir_content[i]: " << temp_curr_dir_content[i] << "\n";
      cout << "dir_entries[temp_curr_dir_content[i]].file_name: " << dir_entries[temp_curr_dir_content[i]].file_name << "\n";
      if (dir_entries[temp_curr_dir_content[i]].file_name == filepath)
      {
        cout << "A file with that name already exists in this directory, try again\n";
        return 0;
      }
    }
    cout << "4\n";
    if (dir_path_temp_temp.size() != 0)
    {
      if (accessrights_check(dir_path_temp_temp.back(), WRITE) == -1)
      {
        cout << "You do not have the rights to create files in this directory! \n";
        return 0;
      }
    }
    cout << "5\n";
    cout << "Enter the information: ";

    while (getline(cin, string_to_eval_temp) && string_to_eval_temp.length() != 0)
    {
      string_to_eval += string_to_eval_temp + "\n";
    }
    // convert string_to_eval to uint8_t*
    uint8_t* block = (uint8_t*)string_to_eval.c_str();

    size_of_file = string_to_eval.length();
    int size_of_file_temp = size_of_file;

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
    struct dir_entry temp_entry;

    //insert data into directory entry
    strcpy(temp_entry.file_name, filepath.c_str());

    temp_entry.size = size_of_file;
    temp_entry.first_blk = start_block;
    temp_entry.type = TYPE_FILE;
    temp_entry.access_rights = READ + WRITE;

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

    for( int i = 0; i < ROOT_SIZE; i++)
    {
      cout << "temp_curr_dir_content[i]: " << temp_curr_dir_content[i] << "\n";
      if (temp_curr_dir_content[i] == -1)
      {
        temp_curr_dir_content[i] = dir_entry_index;
        if(dir_path_temp.size() == dir_path.size() || dir_path_temp == dir_path)
        {
          curr_dir_content[i] = dir_entry_index;
        }
        break;
      }
    }

    disk.write(ROOT_BLOCK, (uint8_t*)dir_entries);

    if(dir_path_temp_temp.size() == 0)
    {
      return 0;
    }

    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT
    /**DONT TOUCH**/ cout << endl; //DONT TOUCH
    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT

    if (dir_path_temp_temp.size() != 0)
    {
      int block_to_write = dir_entries[dir_path_temp_temp.back()].first_blk;
      disk.write(block_to_write, (uint8_t*)temp_curr_dir_content);
    }

    // save_curr_dir();
    std::cout << "FS::create(" << filepath << ")\n";
    return 0;
}


// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath) //KLAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAR
{
    std::cout << "FS::cat(" << filepath << ")\n";
    int blocks_to_read;
    int entry_index;
    int file_found = 0;

    std::vector<int> dir_path_temp = string_to_vector_converter(filepath);
    if (dir_path_temp.size() >= 1)
    {
      if(dir_path_temp.back() == -1)
      {
        cout << "error cant go above root" << endl;
        return -1;
      }
    }
    std::vector<int> dir_path_temp_temp = dir_path;
    int temp_curr_dir_content[BLOCK_SIZE];

    for(int r = 0; r < ROOT_SIZE; r++)
    {
      temp_curr_dir_content[r] = curr_dir_content[r];
    }
    if (dir_path_temp.size() != dir_path.size() || dir_path_temp != dir_path)
    {
      dir_path_temp_temp = dir_path_temp;
      disk.read(dir_entries[dir_path_temp_temp.back()].first_blk, (uint8_t*)temp_curr_dir_content);

      //Ful lösning för att hitta namnet
      std::vector<int> dir_path_name_find = dir_path;
      std::vector<int> path;
      std::vector<string> path_str;
      string dir;
      char delimiter = '/';
      int temp_dir_content[BLOCK_SIZE];
      int filepath_size = filepath.length();
      char filepath_char[filepath_size];
      strcpy(filepath_char, filepath.c_str());

      membuf sbuf(filepath_char, filepath_char + sizeof(filepath_char));
      istream in(&sbuf);

      while (getline(in, dir, delimiter))
      {
        path_str.push_back(dir);
      }
      filepath = path_str.back();

      if (dir_path_temp_temp.size() == 0)
      {
        std::vector<int> root_dir_content;
        root_dir_content = init_dir_content(dir_path_temp_temp);
        for(int i = 0; i < ROOT_SIZE; i++)
        {
           temp_curr_dir_content[i] = root_dir_content[i];
           if(dir_path_temp.size() == dir_path.size())
           {
             curr_dir_content[i] = root_dir_content[i];
           }
        }
      }
    }

    // Find file
    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if(dir_entries[temp_curr_dir_content[i]].file_name == filepath)
      {
        file_found = 1;
        entry_index = temp_curr_dir_content[i];
        blocks_to_read = dir_entries[temp_curr_dir_content[i]].size/BLOCK_SIZE;
        // Add rest block if exists
        if(dir_entries[temp_curr_dir_content[i]].size%BLOCK_SIZE > 0)
        {
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

    if (dir_entries[entry_index].type == TYPE_DIR)
    {
      cout << "Cannot read a directory! \n";
      return 0;
    }

    if (accessrights_check(entry_index, READ) == -1)
    {
      cout << "You do not have the rights to read this file! \n";
      return 0;
    }
    // if type = file
    if(dir_entries[entry_index].type == TYPE_FILE)
    {

      if(blocks_to_read == 0)
      {
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
    }
    return 0;
}

// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls() ////KLAR
{
  cout << "name" << setw(15) << "type" << setw(15) << "accessrights" << setw(15) << "size" << "\n";
  for (int i = 0; i < sizeof(curr_dir_content)/sizeof(curr_dir_content[0]); i++)
      {
        if (curr_dir_content[i] != -1)
        {
          string type;
          if (dir_entries[curr_dir_content[i]].type == TYPE_DIR)
          {
            type = "dir";
          }
          else
          {
            type = "file";
          }

          string accessrights_str;
          int accessrights = dir_entries[curr_dir_content[i]].access_rights;

          if (accessrights == 0)
          {
            accessrights_str = "---";
          }

          else if (accessrights == READ)
          {
            accessrights_str = "r--";
          }

          else if (accessrights == READ + WRITE)
          {
            accessrights_str = "rw-";
          }

          else if (accessrights == READ + WRITE + EXECUTE)
          {
            accessrights_str = "rwx";
          }

          else if (accessrights == READ + EXECUTE)
          {
            accessrights_str = "r-x";
          }

          else if (accessrights == WRITE)
          {
            accessrights_str = "-w-";
          }

          else if (accessrights == WRITE + EXECUTE)
          {
            accessrights_str = "-wx";
          }

          else if (accessrights == EXECUTE)
          {
            accessrights_str = "--x";
          }
          cout << dir_entries[curr_dir_content[i]].file_name << setw(15) << type << setw(15) << accessrights_str << setw(15) << dir_entries[curr_dir_content[i]].size << "\n";
        }
      }

  return 0;
}

// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int
FS::cp(std::string sourcepath, std::string destpath) //KLAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAR
{
    std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";
    int dir_entry_index = -1;
    char name_array[56];

    //SOURCE
    std::vector<int> dir_path_temp_source = string_to_vector_converter(sourcepath);

    if (dir_path_temp_source.size() >= 1)
    {
      if(dir_path_temp_source.back() == -1)
      {
        cout << "error cant go above root" << endl;
        return -1;
      }
    }

    std::vector<int> dir_path_temp_temp_source = dir_path;
    int temp_curr_dir_content_source[BLOCK_SIZE];

    for(int r = 0; r < ROOT_SIZE; r++)
    {
      temp_curr_dir_content_source[r] = curr_dir_content[r];
    }

    if (dir_path_temp_source.size() != dir_path.size() || dir_path_temp_source != dir_path)
    {
      dir_path_temp_temp_source = dir_path_temp_source;
      disk.read(dir_entries[dir_path_temp_temp_source.back()].first_blk, (uint8_t*)temp_curr_dir_content_source);

      //Ful lösning för att hitta namnet
      std::vector<int> dir_path_name_find = dir_path;
      std::vector<int> path;
      std::vector<string> path_str;
      string dir;
      char delimiter = '/';
      int temp_dir_content[BLOCK_SIZE];
      int sourcepath_size = sourcepath.length();
      char sourcepath_char[sourcepath_size];
      strcpy(sourcepath_char, sourcepath.c_str());

      membuf sbuf(sourcepath_char, sourcepath_char + sizeof(sourcepath_char));
      istream in(&sbuf);

      while (getline(in, dir, delimiter))
      {
        path_str.push_back(dir);
      }
      sourcepath = path_str.back();
    }

    //DEST
    std::vector<int> dir_path_temp = string_to_vector_converter(destpath);
    if (dir_path_temp.size() >= 1)
    {
      if(dir_path_temp.back() == -1)
      {
        cout << "error cant go above root" << endl;
        return -1;
      }
    }
    std::vector<int> dir_path_temp_temp = dir_path;
    int temp_curr_dir_content[BLOCK_SIZE];

    for(int r = 0; r < ROOT_SIZE; r++)
    {
      temp_curr_dir_content[r] = curr_dir_content[r];
    }

    if (dir_path_temp.size() != dir_path.size() || dir_path_temp != dir_path)
    {
      dir_path_temp_temp = dir_path_temp;
      disk.read(dir_entries[dir_path_temp_temp.back()].first_blk, (uint8_t*)temp_curr_dir_content);

      //Ful lösning för att hitta namnet
      std::vector<int> dir_path_name_find = dir_path;
      std::vector<int> path;
      std::vector<string> path_str;
      string dir;
      char delimiter = '/';
      int temp_dir_content[BLOCK_SIZE];
      int filepath_size = destpath.length();
      char filepath_char[filepath_size];
      strcpy(filepath_char, destpath.c_str());

      membuf sbuf(filepath_char, filepath_char + sizeof(filepath_char));
      istream in(&sbuf);

      while (getline(in, dir, delimiter))
      {
        path_str.push_back(dir);
      }
      destpath = path_str.back();
    }

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
      if(dir_entries[dir_entry_index].type == TYPE_DIR)
      {
        cout << "Cannot copy a directory." << endl;
        return -1;
      }

    if (accessrights_check(dir_entry_index, READ) == -1)
    {
      cout << "You do not have the rights to read this file! \n";
      return 0;
    }

    if (dir_path_temp_temp.size() != 0)
    {
      if (accessrights_check(dir_path_temp_temp.back(), WRITE) == -1)
      {
        cout << "You do not have the rights to write to this directory \n";
        return 0;
      }
    }

    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if (dir_entries[temp_curr_dir_content_source[i]].file_name == destpath)
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

    //Get data from file
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
    //**''*''*'''*''*''**CREATE FILE copied from CREATE function**''*''*''*

    //get file size
    int start_block;
    string string_to_eval = read_data;

    uint8_t* block = (uint8_t*)string_to_eval.c_str();

    size_of_file = string_to_eval.length();
    int size_of_file_temp = size_of_file;

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
    strcpy(temp_entry.file_name, destpath.c_str());

    temp_entry.size = size_of_file;
    temp_entry.first_blk = start_block;
    temp_entry.type = type;
    temp_entry.access_rights = access_rights;

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

    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if (temp_curr_dir_content[i] == -1)
      {
        temp_curr_dir_content[i] = dir_entry_index;
        if(dir_path_temp == dir_path)
        {
          curr_dir_content[i] = dir_entry_index;
        }
        break;
      }
    }

    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if (temp_curr_dir_content_source[i] == -1)
      {
        temp_curr_dir_content_source[i] = dir_entry_index;
        if(dir_path_temp == dir_path)
        {
          curr_dir_content[i] = dir_entry_index;
        }
        break;
      }
    }

    disk.write(ROOT_BLOCK, (uint8_t*)dir_entries); // dir_entries in the file

    if (dir_path_temp.size() != dir_path.size() || dir_path_temp != dir_path)
    {
      for (int i = 0; i < ROOT_SIZE; i++)
      {
        if (dir_entries[temp_curr_dir_content_source[i]].file_name == destpath)
        {
          temp_curr_dir_content_source[i] = -1;
          if(dir_path_temp_source == dir_path)
          {
            curr_dir_content[i] = -1;
          }
          break;
        }
      }
    }

    if(dir_path_temp_temp.size() == 0)
    {
      return 0;
    }

    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT
    /**DONT TOUCH**/ cout << endl; //DONT TOUCH
    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT

    if (dir_path_temp_temp.size() != 0)
    {
      int block_to_write = dir_entries[dir_path_temp_temp.back()].first_blk;
      disk.write(block_to_write, (uint8_t*)temp_curr_dir_content);
    }

    if (dir_path_temp_temp_source.size() != 0)
    {
      int block_to_write = dir_entries[dir_path_temp_temp_source.back()].first_blk;
      disk.write(block_to_write, (uint8_t*)temp_curr_dir_content_source);
    }

    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destpath) //KLAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAR
{
  std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";

  //SOURCEPATH
  std::vector<int> dir_path_temp_source = string_to_vector_converter(sourcepath);
  if (dir_path_temp_source.size() >= 1)
  {
    if(dir_path_temp_source.back() == -1)
    {
      cout << "error cant go above root" << endl;
      return -1;
    }
  }
  std::vector<int> dir_path_temp_temp_source = dir_path;
  int temp_curr_dir_content_source[BLOCK_SIZE];

  cout << "dir_path_temp_size: " << dir_path_temp_source.size() << "\n";

  cout << "1\n";
  for(int r = 0; r < ROOT_SIZE; r++)
  {
    temp_curr_dir_content_source[r] = curr_dir_content[r];
  }

  if (dir_path_temp_source.size() != dir_path.size() || dir_path_temp_source != dir_path)
  {
    cout << "1.2\n";
    dir_path_temp_temp_source = dir_path_temp_source;
    disk.read(dir_entries[dir_path_temp_temp_source.back()].first_blk, (uint8_t*)temp_curr_dir_content_source);

    //Ful lösning för att hitta namnet
    std::vector<int> dir_path_name_find = dir_path;
    std::vector<int> path;
    std::vector<string> path_str;
    string dir;
    char delimiter = '/';
    int temp_dir_content[BLOCK_SIZE];
    int sourcepath_size = sourcepath.length();
    char sourcepath_char[sourcepath_size];
    strcpy(sourcepath_char, sourcepath.c_str());

    membuf sbuf(sourcepath_char, sourcepath_char + sizeof(sourcepath_char));
    istream in(&sbuf);

    while (getline(in, dir, delimiter))
    {
      path_str.push_back(dir);
    }
    sourcepath = path_str.back();
  }

  //DESTPATH
  std::vector<int> dir_path_temp_dest = string_to_vector_converter(destpath);
  if (dir_path_temp_dest.size() >= 1)
  {
    if(dir_path_temp_dest.back() == -1)
    {
      cout << "error cant go above root" << endl;
      return -1;
    }
  }
  std::vector<int> dir_path_temp_temp_dest = dir_path;
  int temp_curr_dir_content_dest[BLOCK_SIZE];

  cout << "dir_path_temp_size: " << dir_path_temp_dest.size() << "\n";

  cout << "1\n";
  for(int r = 0; r < ROOT_SIZE; r++)
  {
    temp_curr_dir_content_dest[r] = curr_dir_content[r];
  }

  if (dir_path_temp_dest.size() != dir_path.size() || dir_path_temp_dest != dir_path)
  {
      cout << "1.2\n";
    dir_path_temp_temp_dest = dir_path_temp_dest;
    disk.read(dir_entries[dir_path_temp_temp_dest.back()].first_blk, (uint8_t*)temp_curr_dir_content_dest);

    if (dir_path_temp_temp_dest.size() == 0)
    {
        cout << "1.3\n";
      std::vector<int> root_dir_content;
      root_dir_content = init_dir_content(dir_path_temp_temp_dest);
      for(int i = 0; i < ROOT_SIZE; i++)
      {
         cout << "1.4\n";
         temp_curr_dir_content_dest[i] = root_dir_content[i];
      }

      for (int i = 0; i < ROOT_SIZE; i++)
      {
        if (dir_entries[curr_dir_content[i]].file_name == sourcepath)
        {
        curr_dir_content[i] = -1;
        break;
        }
      }

      disk.write(dir_entries[dir_path.back()].first_blk, (uint8_t*)curr_dir_content);
      return 0;
    }
  }

  cout << "2\n";

  //check if sourcepath exists
  int src_entry_index = -1;
  for (int i = 0; i < ROOT_SIZE; i++)
  {
    cout << "dir_entries[curr_dir_content[i]].file_name: " << dir_entries[temp_curr_dir_content_source[i]].file_name << " = " << "sourcepath: " << sourcepath << "\n";
    if (dir_entries[temp_curr_dir_content_source[i]].file_name == sourcepath)
    {
      src_entry_index = temp_curr_dir_content_source[i];
      break;
    }
  }
  if(src_entry_index == -1)
  {
    cout << "source cannot not be found\n";
    return -1;
  }

  cout << "3\n";
  if (destpath == "..")
  {
    if (dir_path_temp_temp_dest.size() == 0)
    {
      cout << "Already in root!\n";
      return 0;
    }

    if(dir_path_temp_temp_dest.size() -1 == 0)
    {
      // make to orphan
      // find index
      int dir_entry_index;
      for(int i = 0; ROOT_SIZE; i++)
      {
        if(dir_entries[temp_curr_dir_content_dest[i]].file_name == sourcepath)
        {
          dir_entry_index = temp_curr_dir_content_dest[i];
          break;
        }
      }

      if (accessrights_check(dir_path_temp_temp_dest.back(), WRITE) == -1)
      {
        cout << "You do not the rights to write to this directory! \n";
        return 0;
      }

      //check if other orphan with same name already exists
      // get orphan lists
      std::vector<int> root_dir_content;
      vector<int> temp_dir_path;
      root_dir_content = init_dir_content(temp_dir_path);
      for (int i = 0; i < ROOT_SIZE; i++)
      {
        if (sourcepath == dir_entries[root_dir_content[i]].file_name)
        {
          cout << "Already an item with the same name in root" << endl;
          return -1;
        }
      }
      cout << "-2 \n";
      for(int i = 0; i < ROOT_SIZE; i++ ) {
        if(temp_curr_dir_content_dest[i] == dir_entry_index)
        {
          temp_curr_dir_content_dest[i] = -1;
          if(dir_path_temp_dest.size() == 0)
          {
            cout << "in root remove! \n";
            curr_dir_content[i] = -1;
          }
          break;
        }
      }
      disk.write(dir_entries[dir_path_temp_temp_dest.back()].first_blk, (uint8_t*)temp_curr_dir_content_dest);
      return 0;
    }

    vector<int> dir_path_temp = dir_path_temp_temp_dest;
    dir_path_temp.resize(dir_path_temp.size()-1);
    // General case
    int dir_entry_index;
    for(int i = 0; ROOT_SIZE; i++)
    {
      if(dir_entries[temp_curr_dir_content_dest[i]].file_name == sourcepath)
      {
        dir_entry_index = temp_curr_dir_content_dest[i];
        break;
      }
    }

    if (accessrights_check(dir_path_temp_temp_dest.back(), WRITE) == -1 || accessrights_check(dir_path_temp_dest.back(), WRITE) == -1)
    {
      cout << "You do not the rights to write to one or both of these directories! \n";
      return 0;
    }
    // get parents dirs
    int parent_dir_content[BLOCK_SIZE];
    disk.read(dir_entries[dir_path_temp.back()].first_blk, (uint8_t*)parent_dir_content);
    //check if parents kids share name
    for (int i = 0; i < ROOT_SIZE; i++)
    {
      if (sourcepath == dir_entries[parent_dir_content[i]].file_name)
      {
        cout << "Already an item with the same name in root" << endl;
        return -1;
      }
    }
    cout << "-1 \n";
    for(int i = 0; i < ROOT_SIZE; i++ )
    {
      if(temp_curr_dir_content_dest[i] == dir_entry_index)
      {
        temp_curr_dir_content_dest[i] = -1;
        if(dir_path_temp_dest.size() == 0)
        {
          cout << "in root remove! \n";
          curr_dir_content[i] = -1;
        }
        break;
      }
    }

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
  cout << "4\n";
  //find index of sourcefile
  int dir_entry_index;
  for (int i = 0; i < ROOT_SIZE; i++) // possible dupe
  {
    if (dir_entries[temp_curr_dir_content_source[i]].file_name == sourcepath)
    {
      dir_entry_index = temp_curr_dir_content_source[i];
      break;
    }
  }
  cout << "5\n";
  // Is destpath a directory?
  // Relative path
  if (dir_path_temp_dest.size() != dir_path.size() || dir_path_temp_dest != dir_path || dir_path_temp_source.size() != dir_path.size() || dir_path_temp_source != dir_path)
  {
    cout << "5.1\n";
    if(dir_path.size() == dir_path_temp_temp_source.size())
    {
      cout << "5.2\n";
      for (int i = 0; i < ROOT_SIZE; i++)
      {
        if (temp_curr_dir_content_dest[i] == -1)
        {
          temp_curr_dir_content_dest[i] = dir_entry_index;
          break;
        }
      }
      disk.write(dir_entries[dir_path_temp_temp_dest.back()].first_blk, (uint8_t*) temp_curr_dir_content_dest);

      for (int i = 0; i < ROOT_SIZE; i++)
      {
        if (curr_dir_content[i] == dir_entry_index)
        {
          curr_dir_content[i] = -1;
          break;
        }
      }
      return 0;
    }

    if(dir_path.size() != dir_path_temp_temp_source.size())
    {
      for (int i = 0; i < ROOT_SIZE; i++)
      {
        if (dir_entries[temp_curr_dir_content_source[i]].file_name == sourcepath)
        {
          dir_entry_index = temp_curr_dir_content_source[i];
          break;
        }
      }
      cout << "5.3\n";
      for (int i = 0; i < ROOT_SIZE; i++)
      {
        if (temp_curr_dir_content_dest[i] == -1)
        {
          cout << "5.3.1\n";
          temp_curr_dir_content_dest[i] = dir_entry_index;
          break;
        }
      }
      cout << dir_path_temp_temp_dest.back() << "\n";
      disk.write(dir_entries[dir_path_temp_temp_dest.back()].first_blk, (uint8_t*) temp_curr_dir_content_dest);

      for (int i = 0; i < ROOT_SIZE; i++)
      {
        if (temp_curr_dir_content_source[i] == dir_entry_index)
        {
          cout << "5.3.2\n";
          temp_curr_dir_content_source[i] = -1;
          break;
        }
      }
      disk.write(dir_entries[dir_path_temp_temp_source.back()].first_blk, (uint8_t*) temp_curr_dir_content_source);
      return 0;
    }
  }

  for(int i = 0; i < ROOT_SIZE; i++) //GÖR OM HELA ?
  {
    cout << "i: " << i << endl;
    if((dir_entries[temp_curr_dir_content_dest[i]].file_name == destpath) && (dir_entries[temp_curr_dir_content_dest[i]].type == TYPE_DIR))
    {
      //**""**""**""**""  enter folder  **""**""**""**""
      int dest_dir_index = temp_curr_dir_content_dest[i];
      cout << "5.1\n";
      //Find folder
      vector<int> dir_path_find = dir_path_temp_temp_dest;

      for(int i = 0; i < dir_path_find.size(); i++)
      {
        cout << "dir_path_find: " << dir_path_find[i] << endl;
        cout << "temp_cur_dir_contetnt_dest: " << temp_curr_dir_content_dest[i] << endl;
      }

      //*""**""**""**""**""GÖR OM DESTPATH TILL INDEX**""**""*""*""*"*"*"*"*
      dir_path_find.push_back(dest_dir_index);
      int dir_entry_index;
      cout << "5.2\n";
      for(int i = 0; ROOT_SIZE; i++)
      {
        if(dir_entries[temp_curr_dir_content_dest[i]].file_name == sourcepath)
        {
          dir_entry_index = temp_curr_dir_content_dest[i];
          break;
        }
      }
      cout << "6\n";
      if (dir_path_temp_temp_dest.size() != 0)
      {
        if (accessrights_check(dir_path_temp_temp_dest.back(), WRITE) == -1)
        {
          cout << "You do not the rights to write to one or both of these directories! \n";
          return 0;
        }
      }
      cout << "7\n";
      if (accessrights_check(dir_path_find.back(), WRITE) == -1)
      {
        cout << "You do not the rights to write to one or both of these directories! \n";
        return 0;
      }
      cout << "8 \n";

      if(dir_path_temp_temp_dest.size() == 0)
      {
              cout << "8.1 \n";
        if (dir_path_temp_dest.size() != dir_path.size() || dir_path_temp_dest != dir_path)
        {
                cout << "8.2 \n";
          for (int i = 0; i < ROOT_SIZE; i++)
          {
            if (dir_entries[curr_dir_content[i]].file_name == sourcepath)
            {
                    cout << "8.2 \n";
            curr_dir_content[i] = -1;
            break;
            }
          }
          disk.write(dir_entries[dir_path.back()].first_blk, (uint8_t*)curr_dir_content);
          return 0;
        }
        cout << "9\n";
        int new_dir_content[BLOCK_SIZE];
        cout << "dir_path_find.back = " << dir_path_find.back() << endl;
        disk.read(dir_entries[dir_path_find.back()].first_blk, (uint8_t*)new_dir_content);

        for(int i = 0; i < ROOT_SIZE; i++)
        {
          cout << new_dir_content[i] << endl;
        }


        // check if name exists in destination
        // for (int i = 0; i < ROOT_SIZE; i++)
        // {
        //   if (sourcepath == dir_entries[new_dir_content[i]].file_name)
        //   {
        //     cout << "Already an item with the same name in root" << endl;
        //     return -1;
        //   }
        // }
        cout << "10 \n";
        for(int i = 0; i < ROOT_SIZE; i++)
        {
          if(curr_dir_content[i] == dir_entry_index)
          {
            curr_dir_content[i] = -1;
            break;
          }
        }
        cout << "11\n";
        for(int i = 0; i < ROOT_SIZE; i++)
        {
          if(new_dir_content[i] == -1)
          {
            new_dir_content[i] = dir_entry_index;
            break;
          }
        }
        cout << "12\n";
        disk.write(dir_entries[dir_path_find.back()].first_blk, (uint8_t*)new_dir_content);

        return 0;
      }

      int new_dir_content[BLOCK_SIZE];
      disk.read(dir_entries[dir_path_find.back()].first_blk, (uint8_t*)new_dir_content);

      // check if name exists in the new directory
      for (int i = 0; i < ROOT_SIZE; i++)
      {
        if (sourcepath == dir_entries[new_dir_content[i]].file_name)
        {
          cout << "Already an item with the same name in root" << endl;
          return -1;
        }
      }
      cout << "3 \n";
      // gotta move after dupe check
      for(int i = 0; i < ROOT_SIZE; i++)
      {
        if(curr_dir_content[i] == dir_entry_index)
        {
          curr_dir_content[i] = -1;
          break;
        }
      }

      disk.write(dir_entries[dir_path_temp_temp_dest.back()].first_blk, (uint8_t*)temp_curr_dir_content_dest);

      for(int i = 0; i < ROOT_SIZE; i++)
      {
        if(new_dir_content[i] == -1)
        {
          new_dir_content[i] = dir_entry_index;
          break;
        }
      }
      disk.write(dir_entries[dir_path_find.back()].first_blk, (uint8_t*)new_dir_content);
      return 0;
    }
  }

  // find source file
  for(int i = 0; i < ROOT_SIZE; i++)
  {
    // check so destpath doesn't exist
    for( int j = 0; j < ROOT_SIZE; j++)
    {
      if(dir_entries[j].file_name == destpath)
      {
        cout << "name already exists or is not a directory" << endl;
        return -1;
      }
    }
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
FS::rm(std::string filepath, int from_append) // KLAAAAAAAAAAAAAAAAAAAAAAAAAAAR
{
    std::vector<int> dir_path_temp = string_to_vector_converter(filepath);
    if (dir_path_temp.size() >= 1)
    {
      if(dir_path_temp.back() == -1)
      {
        cout << "error cant go above root" << endl;
        return -1;
      }
    }
    std::vector<int> dir_path_temp_temp = dir_path;
    int temp_curr_dir_content[BLOCK_SIZE];


    for(int r = 0; r < ROOT_SIZE; r++)
    {
      temp_curr_dir_content[r] = curr_dir_content[r];
    }

    if (dir_path_temp.size() != dir_path.size() || dir_path_temp != dir_path)
    {
      cout << "is relative \n";
      cout << "path temp: " << dir_path_temp.back() << endl;
      //cout << "path temp temp: " << dir_path_temp_temp.back() << endl;
      dir_path_temp_temp = dir_path_temp;
      cout << "path temp temp: " << dir_path_temp_temp.back() << endl;
      cout << "filename: " << dir_entries[dir_path_temp_temp.back()].file_name << endl;
      cout << "blk: "  << dir_entries[dir_path_temp_temp.back()].first_blk << endl;
      // satsen nedan läser in fel block vid mapp sök
      disk.read(dir_entries[dir_path_temp_temp.back()].first_blk, (uint8_t*)temp_curr_dir_content);

      //Ful lösning för att hitta namnet
      std::vector<int> dir_path_name_find = dir_path;
      std::vector<int> path;
      std::vector<string> path_str;
      string dir;
      char delimiter = '/';
      int temp_dir_content[BLOCK_SIZE];
      int filepath_size = filepath.length();
      char filepath_char[filepath_size];
      strcpy(filepath_char, filepath.c_str());

      membuf sbuf(filepath_char, filepath_char + sizeof(filepath_char));
      istream in(&sbuf);

      while (getline(in, dir, delimiter))
      {
        path_str.push_back(dir);
      }
      filepath = path_str.back();
      cout << "filepah: " <<  filepath << endl;
      cout << "dir_path_temp_temp size: " <<  dir_path_temp_temp.size() << endl;
      cout << "dir_path_temp_temp size: " <<  dir_path_temp_temp.size() << endl;
      // IF in root case
      if (dir_path_temp_temp.size() == 0) // if in root
      {
        cout <<"we in root" << endl;
        cout << "1" << endl;
        std::vector<int> root_dir_content;
        root_dir_content = init_dir_content(dir_path);
        int dir_entry_index;
        for (int i = 0; i <  ROOT_SIZE; i++)
        {
          cout << "file name in root: " << dir_entries[root_dir_content[i]].file_name << endl;
          if (dir_entries[root_dir_content[i]].file_name == filepath)
          {
            cout << "found dir ent index of " << filepath << " with id: " << root_dir_content[i] << endl;
            dir_entry_index = root_dir_content[i];
            break;
          }
        }

        int blocks_to_read = dir_entries[dir_entry_index].size/BLOCK_SIZE;
        if(dir_entries[dir_entry_index].size%BLOCK_SIZE > 0)
        {
          blocks_to_read++;
        }

        int curr_block = dir_entries[dir_entry_index].first_blk;
        int index = curr_block;
        for (int i = 0; i < blocks_to_read; i++)
        {
          index = curr_block;
          if(curr_block != -1)
          {
            curr_block = fat[curr_block];
          }
          fat[index] = FAT_FREE;
        }
        cout << "2" << endl;
        // Clear dir entry
        struct dir_entry temp_entry;
        strcpy(temp_entry.file_name, "");
        temp_entry.size = 0;
        temp_entry.first_blk = 0;
        temp_entry.type = 0;
        temp_entry.access_rights = 0;
        dir_entries[dir_entry_index] = temp_entry;

        disk.write(FAT_BLOCK, (uint8_t*)fat);
        disk.write(ROOT_BLOCK, (uint8_t*)dir_entries);
        return 0;
      }
    }

    if (from_append != 2)
    {
      std::cout << "FS::rm(" << filepath << ")\n";
    }

    // is not relative
    cout << "3\n";
    int name_found = 0;
    int dir_entry_index;
    for(int i = 0; i < ROOT_SIZE; i++)
    {
      cout << dir_entries[temp_curr_dir_content[i]].file_name << " = " << filepath << endl;
      if(dir_entries[temp_curr_dir_content[i]].file_name == filepath) // update: bytte ut sök på man till sök på dir index
      {
        cout << "2\n";
        name_found = 1;
        dir_entry_index = temp_curr_dir_content[i];

        // if folder with content, throw error
        if(dir_entries[dir_entry_index].type == TYPE_DIR)
        {
          cout << "3\n";
          int temp_dir_content[BLOCK_SIZE];
          cout << "looking at temp_curr_dir_content of " << dir_entries[dir_entry_index].file_name << endl;
          disk.read(dir_entries[dir_entry_index].first_blk, (uint8_t*)temp_dir_content);
          for(int j = 0; j < ROOT_SIZE; j++) // Ska inte kollas om det en relativ path!!
          {
            if (temp_dir_content[j] != -1)
            {
              cout << "4\n";
              cout << "Empty the directory before removing it." << endl;
              return -1;
            }
          }
        }

        if (accessrights_check(dir_entry_index, WRITE) == -1 && from_append == 1)
        {
          cout << "You do not have the rights to write to this file! \n";
          return 0;
        }

        if (dir_path_temp_temp.size() != 0)
        {
          if (accessrights_check(dir_path_temp_temp.back(), WRITE) == -1 && from_append == 1)
          {
            cout << "You do not have the rights to write to this directory! \n";
            return 0;
          }
        }

        // Clear fat
        int blocks_to_read = dir_entries[temp_curr_dir_content[i]].size/BLOCK_SIZE;
        if(dir_entries[temp_curr_dir_content[i]].size%BLOCK_SIZE > 0)
        {
          blocks_to_read++;
        }

        int curr_block = dir_entries[temp_curr_dir_content[i]].first_blk;
        int index = curr_block;
        for (int i = 0; i < blocks_to_read; i++)
        {
          index = curr_block;
          if(curr_block != -1)
          {
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
        dir_entries[temp_curr_dir_content[i]] = temp_entry;

        for (int j = 0; j < ROOT_SIZE; j++)
        {
          if (temp_curr_dir_content[j] == dir_entry_index)
          {
            temp_curr_dir_content[j] = -1;
            if(dir_path_temp.size() == 0)
            {
              curr_dir_content[i] = -1;
            }
            break;
          }
        }
      }
    }

    if (name_found == 0)
    {
      cout << "No file with that name found, try again\n";
      return 0;
    }

    // Save curr dir
    int curr_dir_id;

    if(dir_path_temp_temp.size() == 0)
    {
      cout << "im here? \n";
      disk.write(FAT_BLOCK, (uint8_t*)fat);
      disk.write(ROOT_BLOCK, (uint8_t*)dir_entries);
      return 0;
    }

    // if remove folder, shall we remove content? NO!!!
    int parent_dir_content[BLOCK_SIZE];
    disk.read(dir_entries[dir_path_temp_temp.back()].first_blk, (uint8_t*)parent_dir_content);

    for (int i = 0; i < ROOT_SIZE; i++)
    {
      if(parent_dir_content[i] == dir_entry_index)
      {
        parent_dir_content[i] = -1;
        temp_curr_dir_content[i] = -1; //stoopid
        if(dir_path_temp == dir_path)
        {
          curr_dir_content[i] = -1;
        }
        break;
      }
    }
    disk.write(dir_entries[dir_path_temp_temp.back()].first_blk, (uint8_t*)parent_dir_content);
    disk.write(ROOT_BLOCK, (uint8_t*)dir_entries); // dir_entries in the file
    disk.write(FAT_BLOCK, (uint8_t*)fat); // Fat in the file
    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int
FS::append(std::string filepath1, std::string filepath2) //funkar i root
{
    std::cout << "FS::append(" << filepath1 << "," << filepath2 << ")\n";
    string filepath2_backup = filepath2;
    string filepath1_backup = filepath1;
    //FILE 1
    std::vector<int> dir_path_temp1 = string_to_vector_converter(filepath1);
    // cout << "saijs " << dir_path_temp1.size() << endl;
    if (dir_path_temp1.size() >= 1)
    {
      if(dir_path_temp1.back() == -1)
      {
        cout << "error cant go above root" << endl;
        return -1;
      }
    }
    std::vector<int> dir_path_temp_temp1 = dir_path;
    int temp_curr_dir_content1[BLOCK_SIZE];

    cout << "1\n";

    for(int r = 0; r < ROOT_SIZE; r++)
    {
      temp_curr_dir_content1[r] = curr_dir_content[r];
    }

    if (dir_path_temp1.size() != dir_path.size() || dir_path_temp1 != dir_path)
    {
      dir_path_temp_temp1 = dir_path_temp1;
      disk.read(dir_entries[dir_path_temp_temp1.back()].first_blk, (uint8_t*)temp_curr_dir_content1);

      //Ful lösning för att hitta namnet
      std::vector<int> dir_path_name_find = dir_path;
      std::vector<int> path;
      std::vector<string> path_str;
      string dir;
      char delimiter = '/';
      int temp_dir_content[BLOCK_SIZE];
      int filepath1_size = filepath1.length();
      char filepath1_char[filepath1_size];
      strcpy(filepath1_char, filepath1.c_str());

      membuf sbuf(filepath1_char, filepath1_char + sizeof(filepath1_char));
      istream in(&sbuf);

      while (getline(in, dir, delimiter))
      {
        path_str.push_back(dir);
      }
      filepath1 = path_str.back();
    }
    cout << "2\n";
    //FILE 2
    std::vector<int> dir_path_temp2 = string_to_vector_converter(filepath2);
    cout << "CONVERTED FILE TVå" << endl;
    if (dir_path_temp2.size() >= 1)
    {
      if(dir_path_temp2.back() == -1)
      {
        cout << "error cant go above root" << endl;
        return -1;
      }
    }
    std::vector<int> dir_path_temp_temp2 = dir_path;
    int temp_curr_dir_content2[BLOCK_SIZE];

    for(int r = 0; r < ROOT_SIZE; r++)
    {
      temp_curr_dir_content2[r] = curr_dir_content[r];
    }
    cout << "dirpath temp 2 sajs: " << dir_path_temp2.size() << endl;
    // cout << "dirpath temp 2 back: " << dir_path_temp2.back() << endl;
    cout << "dirpath sajs: " << dir_path.size() << endl;

    if (dir_path_temp2.size() != dir_path.size() || dir_path_temp2 != dir_path)
    {
      cout << "Entered is relative path2" << endl;
      dir_path_temp_temp2 = dir_path_temp2;
      disk.read(dir_entries[dir_path_temp_temp2.back()].first_blk, (uint8_t*)temp_curr_dir_content2);
      //Ful lösning för att hitta namnet
      std::vector<int> dir_path_name_find = dir_path;
      std::vector<int> path;
      std::vector<string> path_str;
      string dir;
      char delimiter = '/';
      int temp_dir_content[BLOCK_SIZE];
      int filepath2_size = filepath2.length();
      char filepath2_char[filepath2_size];
      strcpy(filepath2_char, filepath2.c_str());

      membuf sbuf(filepath2_char, filepath2_char + sizeof(filepath2_char));
      istream in(&sbuf);

      while (getline(in, dir, delimiter))
      {
        path_str.push_back(dir);
      }
      filepath2 = path_str.back();
      cout <<"filepath 2: " << filepath2 << endl;
    }
    cout <<"filepath 2 AGAIN: " << filepath2 << endl;
    cout << "3\n";
    // *""*""*""*""*"" FIND FILES *""*""*""*""*""**
    //File 1
    int dir_entry_index_1 = -1;
    int blocks_to_read_1;
    int entry_index_1;
    int file_found_1 = 0;

    // Find file source
    for(int i = 0; i < ROOT_SIZE; i++){

      if(dir_entries[temp_curr_dir_content1[i]].file_name == filepath1)
      {
            cout << "4\n";
        file_found_1 = 1;
        entry_index_1 = temp_curr_dir_content1[i];
        // check if dir

        if(dir_entries[entry_index_1].type == TYPE_DIR)
        {
          cout << "Cannot not append directory." << endl;
          return -1;
        }
        blocks_to_read_1 = dir_entries[temp_curr_dir_content1[i]].size/BLOCK_SIZE;
        dir_entry_index_1 = temp_curr_dir_content1[i];
            cout << "5\n";
        // Add rest block if exists
        if(dir_entries[temp_curr_dir_content1[i]].size%BLOCK_SIZE > 0)
        {
          blocks_to_read_1++;
        }
        break;
      }
    }
    cout << "6\n";
    if (file_found_1 == 0)
    {
      cout << "No file with that name found, try again\n";
      return 0;
    }

    if(blocks_to_read_1 == 0)
    {
      return -1;
    }

    //File 2
    int dir_entry_index_2 = -1;
    int blocks_to_read_2;
    int entry_index_2;
    int file_found_2 = 0;
    cout << "6.1\n";
    // Find file

    for(int i = 0; i < ROOT_SIZE; i++)
    {
      if(dir_entries[temp_curr_dir_content2[i]].file_name == filepath2)
      {
        cout << "6.2\n";
        file_found_2 = 1;
        entry_index_2 = temp_curr_dir_content2[i];
        // check if dir
        if(dir_entries[entry_index_2].type == TYPE_DIR)
        {
          cout << "Cannot not append directory." << endl;
          return -1;
        }
                cout << "6.3\n";
        blocks_to_read_2 = dir_entries[temp_curr_dir_content2[i]].size/BLOCK_SIZE;
        dir_entry_index_2 = temp_curr_dir_content2[i];
                cout << "6.4\n";
        // Add rest block if exists
        if(dir_entries[temp_curr_dir_content2[i]].size%BLOCK_SIZE > 0)
        {
          blocks_to_read_2++;
        }
        break;
      }
    }
            cout << "7\n";
    if (file_found_2 == 0)
    {
      cout << "No file with that name found, try again\n";
      return 0;
    }

    if(blocks_to_read_2 == 0)
    {
      return -1;
    }
            cout << "8\n";
    if (accessrights_check(dir_entry_index_1, WRITE) == -1 || accessrights_check(dir_entry_index_2, WRITE) == -1)
    {
      cout << "You do not have the rights to write to one or both of these files! \n";
      return 0;
    }

    if (accessrights_check(dir_entry_index_1, READ) == -1 || accessrights_check(dir_entry_index_2, READ) == -1)
    {
      cout << "You do not have the rights to read one or both of these files! \n";
      return 0;
    }
            cout << "9\n";
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
            cout << "10\n";
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
            cout << "11\n";
    // *""*""*""*""*"" APPEND DATA *""*""*""*""*""**
    strcat(read_data_2, read_data_1);

    // *""*""*""*""*"" ADD NEW FILE 2 *""*""*""*""*""**

    //get file size
    int start_block;
    string string_to_eval = read_data_2;
    uint8_t* block = (uint8_t*)string_to_eval.c_str();

    int size_of_file = string_to_eval.length();
    int size_of_file_temp = size_of_file;

    // check amount of block
    int num_blocks = size_of_file / BLOCK_SIZE + 1;
    int num_blocks_temp = size_of_file / BLOCK_SIZE;

    if (size_of_file==BLOCK_SIZE*num_blocks_temp)
    {
      num_blocks = num_blocks_temp;
    }
            cout << "12\n";
    int fitcheck = file_fit_check(num_blocks);

    if (fitcheck == 0)
    {
      cout << "File too large!\n";
      return 0;
    }

    // *""*""*""*""*"" REMOVE FILE 2 *""*""*""*""*""**
    cout << "calling rm from append, file path: "<< filepath2 << endl;

    ls();
    rm(filepath2_backup , 2); // skicka in den hela pathen, inte ombyggda
    ls();

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
                cout << "13\n";
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

    cout << "14\n";
    if (dir_path_temp2 == dir_path)
    {
      for( int i = 0; i < ROOT_SIZE; i++)
      {
        cout << "i: " << i << endl;
        if (temp_curr_dir_content2[i] == -1)
        {
          temp_curr_dir_content2[i] = dir_entry_index;
          if(dir_path_temp2.size() == 0)
          {
            curr_dir_content[i] = dir_entry_index;
          }
          break;
        }
      }
    }

    disk.write(ROOT_BLOCK, (uint8_t*)dir_entries);

    cout << "dir_path_temp2 sajs: "<< dir_path_temp_temp2.size() << endl;
    if(dir_path_temp_temp2.size() == 0)
    {

      return 0;
    }
    cout << "15\n";
    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT
    /**DONT TOUCH**/ cout << endl; //DONT TOUCH
    //REMOVING THIS PRINT CAUSES THE CODE TO SEGFAULT
    cout << "gonna save changes \n";
    cout << dir_path_temp_temp2.back() << endl;
    cout << dir_path_temp_temp2.size() << endl;
    for(int i =  0; i < ROOT_SIZE; i++)
    {
      cout << temp_curr_dir_content2[i] << endl;
    }
    if (dir_path_temp_temp2.size() != 0)
    {

      int block_to_write = dir_entries[dir_path_temp_temp2.back()].first_blk;
      disk.write(block_to_write, (uint8_t*)temp_curr_dir_content2);
    }
    return 0;
}

int
FS::mkdir(std::string dirpath) //Probably klar
{
    std::cout << "FS::mkdir(" << dirpath << ")\n";
    int start_block;
    int num_blocks = 1;
    cout << "1\n";
    std::vector<int> dir_path_temp = string_to_vector_converter(dirpath);
  if (dir_path_temp.size() >= 1)
  {
    if(dir_path_temp.back() == -1)
    {
      cout << "error cant go above root" << endl;
      return -1;
    }
  }
    std::vector<int> dir_path_temp_temp = dir_path;
    int temp_curr_dir_content[BLOCK_SIZE];
    cout << "2\n";
    for(int r = 0; r < ROOT_SIZE; r++)
    {
      temp_curr_dir_content[r] = curr_dir_content[r];
    }
    cout << "3\n";
    if (dir_path_temp.size() != dir_path.size() || dir_path_temp != dir_path)
    {
      cout << "not in root in mkdir\n";
      dir_path_temp_temp = dir_path_temp;
      disk.read(dir_entries[dir_path_temp_temp.back()].first_blk, (uint8_t*)temp_curr_dir_content);

      //Ful lösning för att hitta namnet
      std::vector<int> dir_path_name_find = dir_path;
      std::vector<int> path;
      std::vector<string> path_str;
      string dir;
      char delimiter = '/';
      int temp_dir_content[BLOCK_SIZE];
      int dirpath_size = dirpath.length();
      char dirpath_char[dirpath_size];
      strcpy(dirpath_char, dirpath.c_str());

      membuf sbuf(dirpath_char, dirpath_char + sizeof(dirpath_char));
      istream in(&sbuf);

      while (getline(in, dir, delimiter))
      {
        path_str.push_back(dir);
      }
      dirpath = path_str.back();

    }


    cout << "4\n";
    int fitcheck = file_fit_check(num_blocks);

    if (fitcheck == 0)
    {
      cout << "Disk full!\n";
      return 0;
    }
    cout << "5\n";

    // check if name already exists in directory
    for (int i = 0; i < ROOT_SIZE; i++)
    {
          cout << temp_curr_dir_content[i] << "\n";
      if(dir_entries[temp_curr_dir_content[i]].file_name == dirpath)
      {
        cout << "Name already exists"<< endl;
        return -1;
      }
    }
    cout << "6\n";
    if (dir_path_temp_temp.size() != 0)
    {
      if (accessrights_check(dir_path_temp_temp.back(), WRITE) == -1)
      {
        cout << "You do not have the rights to write to this directory! \n";
        return 0;
      }
    }
    cout << "7\n";
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
          start_block = i - free_space_counter + 1;
          //set fat values
          int elements_in_fat_counter = 1;
          for(int j = start_block; j < start_block + num_blocks; j++)
          {
            if(elements_in_fat_counter == free_space_counter)
            {
              fat[j] = FAT_EOF;
              elements_in_fat_counter++;
            }

            else
            {
              fat[j] = free_spaces[elements_in_fat_counter];
              elements_in_fat_counter++;
            }
          }
          disk.write(FAT_BLOCK, (uint8_t*)fat);
          // write dir entry list
          struct dir_entry temp_entry;
          // write content list to disk
          int temp_dir_content[ROOT_SIZE];
          for(int i = 0; i < ROOT_SIZE; i++)
          {
            temp_dir_content[i] = -1;
          }

          strcpy(temp_entry.file_name, dirpath.c_str());

          temp_entry.first_blk = start_block;
          temp_entry.type = TYPE_DIR;
          temp_entry.access_rights = READ + WRITE;
          temp_entry.size = 0;

          // dir_entry to root block
          //find empty slot in dir entries
          int dir_entry_index;
          for(int r = 0; r < ROOT_SIZE; r++)
          {
            if(dir_entries[r].first_blk == 0)
            {
              dir_entries[r] = temp_entry;
              dir_entry_index = r;
              break;
            }
          }

          int parent_index = dir_path_temp_temp.back();
          int parent_first_blk = dir_entries[parent_index].first_blk;

          if (dir_path_temp_temp.size() != 0)
          {
            for (int j = 0; j < ROOT_SIZE; j ++)
            {
              if (temp_curr_dir_content[j] == -1)
              {
                temp_curr_dir_content[j] = dir_entry_index;
                if(dir_path_temp.size() == dir_path.size())
                {
                  curr_dir_content[i] = dir_entry_index;
                }

                break;
              }
            }
            disk.write(parent_first_blk, (uint8_t*)temp_curr_dir_content);
          }
          disk.write(ROOT_BLOCK, (uint8_t*)dir_entries);
          disk.write(start_block,  (uint8_t*)temp_dir_content);

          if (dir_path_temp_temp.size() == 0)
          {
            std::vector<int> root_dir_content;
            root_dir_content = init_dir_content(dir_path_temp_temp);

            for(int i = 0; i < ROOT_SIZE; i++)
            {
               temp_curr_dir_content[i] = root_dir_content[i];
               if(dir_path_temp.size() == dir_path.size())
               {
                 curr_dir_content[i] = root_dir_content[i];
               }
            }
          }
          break;
        }
      }
    return 0;
}

//  <dirpath> changes the current (working) directory to the directory named <dirpath>
int
FS::cd(std::string new_dir) //KLAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAR
{
  std::cout << "FS::cd(" << new_dir << ")\n";

  // std::vector<int> dir_path_temp = string_to_vector_converter(new_dir);
  // if (dir_path_temp.size() != 0)
  // {
  //   cout << "1\n";
  //   for(int i = 0; i < dir_path_temp.size(); i++)
  //   {
  //     cout << dir_path_temp[i] << "\n";
  //   }
  //
  //   int new_dir_content[BLOCK_SIZE];
  //   disk.read(dir_entries[dir_path_temp.back()].first_blk, (uint8_t*)new_dir_content);
  //
  //   for (int i = 0; i < ROOT_SIZE; i++)
  //   {
  //     curr_dir_content[i] = new_dir_content[i];
  //   }
  //   dir_path = dir_path_temp;
  //   return 0;
  // }
  // cout << "1\n";
  std::vector<int> dir_path_temp = string_to_vector_converter(new_dir, 0, 1);
  if (dir_path_temp.size() >= 1)
  {
    if(dir_path_temp.back() == -1)
    {
      cout << "error cant go above root" << endl;
      return -1;
    }
  }

  std::vector<int> dir_path_temp_temp = dir_path;
  int temp_curr_dir_content[BLOCK_SIZE];

  for(int r = 0; r < ROOT_SIZE; r++)
  {
    temp_curr_dir_content[r] = curr_dir_content[r];
  }
    // cout << "2\n";

  if (dir_path_temp.size() != dir_path.size() || dir_path_temp != dir_path)
  {
    dir_path_temp_temp = dir_path_temp;
      // cout << "2.2\n";
    disk.read(dir_entries[dir_path_temp_temp.back()].first_blk, (uint8_t*)temp_curr_dir_content);
    new_dir = dir_entries[dir_path_temp_temp.back()].file_name;

    if (dir_path_temp_temp.size() == 0)
    {
      cout << "dir_path_temp_temp.size: " << dir_path_temp_temp.size() << "\n";
      std::vector<int> root_dir_content;
      root_dir_content = init_dir_content(dir_path_temp_temp);

      for(int i = 0; i < ROOT_SIZE; i++)
      {
          curr_dir_content[i] = root_dir_content[i];
      }
      dir_path = dir_path_temp_temp;
      return 0;
    }
    // cout << "dir_path_temp_temp.size(): " << dir_path_temp_temp.size() << "\n";
  }

  for (int i = 0; i < dir_path_temp_temp.size(); i++)
  {
    // cout << "dir_path_temp_temp: " << dir_path_temp_temp[i] << "\n";
    // cout << "dir_path: " << dir_path[i] << "\n";
  }

  for(int r = 0; r < ROOT_SIZE; r++)
  {
    // cout << "temp_curr_dir_content[r]: " << temp_curr_dir_content[r] << "\n";
  }

    // cout << "3\n";
  // cout << dir_path_temp.size() << "\n";
  if (new_dir == "..")
  {
    // cout << "4\n";
  // if new directory is ".." then we go up one level.
    if(dir_path_temp_temp.size() == 0)
    {
      // cout << "Can't go futher up, you're at root. \n";
      return -1;
    }

    if (dir_path_temp_temp.size() - 1 == 0)
    {
      std::vector<int> root_dir_content;
      dir_path_temp_temp.resize(dir_path_temp_temp.size() - 1);
      root_dir_content = init_dir_content(dir_path_temp_temp);
        // cout << "5\n";
      for(int i = 0; i < ROOT_SIZE; i++)
      {
        temp_curr_dir_content[i] = root_dir_content[i];
          // cout << "6\n";
        if(dir_path_temp.size() == dir_path.size())
        {
          curr_dir_content[i] = root_dir_content[i];
            // cout << "7\n";
        }
      }

      for(int i = 0; i < dir_path_temp_temp.size(); i++)
      {
        cout << dir_path_temp_temp[i] << "\n";
          // cout << "8\n";
      }

      dir_path = dir_path_temp_temp;
      return 0;
    }

    dir_path_temp_temp.resize(dir_path_temp_temp.size() - 1);
    // Set new folder dir content

    int new_dir_content[BLOCK_SIZE];
    for (int i = 0; i < ROOT_SIZE; i++)
    {
      new_dir_content[i] = -1;
      temp_curr_dir_content[i] = -1;
      if(dir_path_temp.size() == dir_path.size())
      {
        curr_dir_content[i] = -1;
      }
    }

    disk.read(dir_entries[dir_path_temp_temp.back()].first_blk, (uint8_t*)new_dir_content);

    for (int i = 0; i < ROOT_SIZE; i++)
    {
      temp_curr_dir_content[i] = new_dir_content[i];
      if(dir_path_temp.size() == dir_path.size())
      {
        curr_dir_content[i] = new_dir_content[i];
      }
    }

    dir_path = dir_path_temp_temp;
    return 0;
  }

  else
  {

    if (dir_path_temp.size() != dir_path.size() || dir_path_temp != dir_path)
    {
      // cout << "3.1\n";
      if (accessrights_check(dir_path_temp_temp.back(), READ) == -1)
      {
        cout << "You do not have the rights to read this directory! \n";
        return 0;
      }


      for(int i = 0; i < ROOT_SIZE; i++)
      {
        curr_dir_content[i] = temp_curr_dir_content[i];
      }
      for (int i = 0; i < dir_path_temp_temp.size(); i++)
      {
        // cout << "dir_path_temp_temp: " << dir_path_temp_temp[i] << "\n";
        // cout << "dir_path: " << dir_path[i] << "\n";
      }
      dir_path = dir_path_temp_temp;
      return 0;
    }

    // cout << "3\n";
    for (int i = 0; i < ROOT_SIZE; i++)
    {
      cout << dir_entries[temp_curr_dir_content[i]].file_name <<" = " << new_dir <<"\n";
      if ((dir_entries[temp_curr_dir_content[i]].file_name == new_dir) && (dir_entries[temp_curr_dir_content[i]].type == TYPE_DIR))
      {
        // cout << "3.1\n";
        if (accessrights_check(temp_curr_dir_content[i], READ) == -1)
        {
          cout << "You do not have the rights to read this directory! \n";
          return 0;
        }
        // cout << "4\n";
        dir_path_temp_temp.resize(dir_path_temp_temp.size() + 1);
        dir_path_temp_temp[dir_path_temp_temp.size() -1 ] = temp_curr_dir_content[i];

        int new_dir_content[BLOCK_SIZE];
        for (int i = 0; i < ROOT_SIZE; i++)
        {
          new_dir_content[i] = -1;
          temp_curr_dir_content[i] = -1;
          if(dir_path_temp.size() == dir_path.size())
          {
            curr_dir_content[i] = -1;
          }
        }
        // cout << "5\n";
        disk.read(dir_entries[dir_path_temp_temp.back()].first_blk, (uint8_t*)new_dir_content);

        for(int i = 0; i < ROOT_SIZE; i++)
        {
          temp_curr_dir_content[i] = new_dir_content[i];
          if(dir_path_temp.size() == dir_path.size())
          {
            curr_dir_content[i] = new_dir_content[i];
          }
        }
        // cout << "6\n";
        dir_path = dir_path_temp_temp;
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

    for(int i = 0; i < ROOT_SIZE; i++)
    {
      cout << "fat: "<< fat[i] << endl;
    }

    for(int i = 0; i < ROOT_SIZE; i++)
    {
    cout << "dir_name: " << dir_entries[i].file_name << endl;
    }


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

//KVAR ATT IMPLEMENTERA
//1. chmod
//2. pwd
//3.
