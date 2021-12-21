#include <iostream>
#include "fs.h"
#include <cstring>
#include <vector>
using namespace std; // Check if used in datorsalen
//check includes in datorsalen
FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
}

FS::~FS()
{

}

// formats the disk, i.e., creates an empty file system
int
FS::format()
{
    uint8_t* empty_block = 0;
 
    for (int i = 0; i >= disk.get_no_blocks(); i++)
    {
      fat[i] = FAT_FREE;
      disk.write(i, empty_block);
    }
    dir_entry *temp_entry = new dir_entry;
    dir_entry dir_entries[BLOCK_SIZE / sizeof(struct dir_entry)]; // Since we're limited to 1 block, we can only use 64 dir_entries
    
    fat[ROOT_BLOCK] = 1;
    disk.write(ROOT_BLOCK, (uint8_t*)&dir_entries); // dir_entries at the file

    fat[FAT_BLOCK] = 1; 
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
    cout << "Enter the information: ";

    while (getline(cin, string_to_eval_temp) && string_to_eval_temp.length() != 0)
    {
      string_to_eval += string_to_eval_temp + "\n";
    }
    // convert string_to_eval to uint8_t*
    uint8_t* block = (uint8_t*)string_to_eval.c_str();


    size_of_file = string_to_eval.length();

    //insert data into directory entry

    // check amount of block
    int num_blocks = size_of_file / BLOCK_SIZE;
    // Always 1 atleast
    if(num_blocks == 0) {
      num_blocks = 1;
    }   

    cout << "num blocks: " << num_blocks << "\n";
    int free_spaces = 0;
    cout << "Start for loop\n";
    for (int i = 0; i < BLOCK_SIZE/2; i++) {
      // check where the file can fit in the fat
      if (fat[i] == FAT_FREE) {
        cout << "Free space at: " << i << "\n";
        free_spaces++;
      } else {
        free_spaces = 0;
      }

      if (free_spaces >= num_blocks) {
        cout << "Found space\n";
        // go back and fill
        start_block = i - free_spaces + 1;
        cout << "Start block: " << start_block << "\n";

        for(int j = start_block; j < start_block + num_blocks; j++) {
          fat[j] = FAT_BLOCK;
          //TODO SPLIT BLOCK DATA IF TO LARGE
          cout << "writing to disk\n";
          disk.write(j, block);
        }

        
        cout << "finished to write \n";
        break;
      }

    }

    cout << "End loop\n";

    dir_entry *temp_entry = new dir_entry;

    strcpy(temp_entry->file_name, filepath.c_str());
    
    temp_entry->size = size_of_file;
    temp_entry->first_blk = start_block;
    temp_entry->type = TYPE_FILE;
    temp_entry->access_rights = READ;
    // entry to root
    //disk.write(ROOT_BLOCK);
   
    //cout << "struct size: " << sizeof(struct dir_entry) << "\n";
    std::cout << "FS::create(" << filepath << ")\n";
    return 0;
}

// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath)
{
    std::cout << "FS::cat(" << filepath << ")\n";

    string lineText;
    // read from txt file
    ifstream readFile(filepath);

    //get line by line
    while(getline(readFile, lineText)) {
      cout << lineText + "\n";
    }

    readFile.close();

    return 0;
}


// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
    std::cout << "FS::ls()\n";
 /*   DIR *dir;
    struct dirent *ent;
    if((dir = opendir("./")) != NULL) {
      // print all the dir stuff
      while ((ent = readdir(dir)) != NULL) {
        printf ("%s    %d\n", ent->d_name, sizeof(ent)); // file size är fel här, MEN! när vi har vårat egna filsystem bör vi kunna checka på FAT:en.
      }
      closedir(dir);
    } else {
      // no open :((
      perror("");
      return EXIT_FAILURE;
    }*/

    return 0;
}

// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int
FS::cp(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";
    return 0;
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
    return 0;
}
