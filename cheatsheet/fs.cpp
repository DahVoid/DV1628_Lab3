#include <iostream>
#include <cstring>
#include "fs.h"

dir_entry *fileInfo = new dir_entry;

vector<dir_entry*> v;
struct str{
  char ch[37];
  int number;
};
//Here we read the metadata from the disk space at a given block that we then return as a vector
std::vector<dir_entry*> FS::readMetaData(int blkPos)
{
  // 64 values because a fat block can only hold 64 structs in it's memory
  dir_entry *arr = new dir_entry[64];
  std::vector<dir_entry*> v;
  // read at blkPos
  disk.read(blkPos, (uint8_t*)arr);
  // dirSize is how many elements there were in the array
  dirSize = arr[0].size;
  for (size_t i = 0; i < arr[0].size; i++) {
    // add the elements from the disk to a global vector
    v.push_back(&arr[i+1]);

  }
  return v;
}
//Here we write the metadata that we have the given vector to a block in the fat array and to the disk space
int FS::writeMetaData(int blkPos, std::vector<dir_entry*> vec1)
{
  // this is here because i'm scared to remove it.
  for (auto &el : vec1)
  {
    cout << ""; // this is only here to actually make loop run
  }
  // 64 values because a fat block can only hold 64 structs in it's memory
  dir_entry arr[64];
  // arr[0].size signifies how many structs will be in the array. Have to do this because an array doesn't know it's size
  arr[0].size = vec1.size();

  for (size_t i = 0; i < vec1.size(); i++) {
    arr[i+1] = *vec1[i];
  }
  disk.write(blkPos, (uint8_t*)&arr);
  return 0;
}

FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
    currentBlk = ROOT_BLOCK;
    currentNode = 0;
    v = readMetaData(currentBlk);

    disk.read(1, (uint8_t*)&fat);
    path.push_back("/");
    bool exit = false;
    // create the base root node
    Node *node = new Node;
    node->entries = readMetaData(ROOT_BLOCK);
    string s = "root";
    strcpy(node->name, s.c_str());
    node->depth = ROOT_BLOCK;
    node->block = ROOT_BLOCK;
    node->parent = node;

    nodes.push_back(node);
    // no point building if there are no directories or files
    if (v.size() >= 1) {
      buildTree(node, node->depth + 1);
    }
}

FS::~FS()
{

}

// formats the disk, i.e., creates an empty file system
int FS::format()
{
    // we have to format the FAT
    std::cout << "FS::format()\n";

    int i = 0;
    v = readMetaData(ROOT_BLOCK);

    fat[0] = ROOT_BLOCK;
    fat[1] = FAT_BLOCK;
    for(i=2; i<BLOCK_SIZE/2;i++)
    {
      fat[i] = FAT_FREE;
    }
    disk.write(1, (uint8_t*)&fat);
    v.clear();
    nodes.clear();

    // create the root again
    Node *node = new Node;
    node->entries = readMetaData(ROOT_BLOCK);
    string s = "root";
    strcpy(node->name, s.c_str());
    node->depth = ROOT_BLOCK;
    node->block = ROOT_BLOCK;
    node->parent = node;
    nodes.push_back(node);

    currentBlk = ROOT_BLOCK;
    currentNode = 0;
    writeMetaData(ROOT_BLOCK, v);
    return 0;
}

// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int FS::create(std::string filepath)
{
  std::cout << "FS::create(" << filepath << ")\n";

  // split dirpath with '/' as divider
  std::vector<string> vec_filepath = split(filepath);
  if (vec_filepath.back().length() >= 56) {
    cout << "ERROR: name for new file is too large" << endl;
    return 0;
  }
  currentNode = travelNode(vec_filepath, MAKE, filepath.at(0));
  // set v to be the data where the user inputted the end of the path
  v = readMetaData(nodes[currentNode]->block);

  // if filepath can be found in the vector of the current folder
  //then return an error message and return to function state
  for (size_t i = 0; i < v.size(); i++) {
    if (vec_filepath.back() == v[i]->file_name)
    {
      cout << "ERROR: you have tried to create a file with the same name as an existing element" << endl;
      return 0;
    }
  }
  string filename = vec_filepath.back();

  dir_entry *dir = new dir_entry();
  dir->access_rights = ( WRITE | READ);
  v.push_back(dir);
  //Enter the given filename to the last element in the vector
  strcpy(v.back()->file_name, filename.c_str());
  uint8_t* t8 = new uint8_t;
  unsigned char *read = new unsigned char[BLOCK_SIZE];
  // the arrays are this big so a user can input up to 100 000 characters
  char ch[100000];
  char ch2[100000];
  string str;
  string str2;
  int size;
  string strTemp;

  //read each line and gather all output in a string and copy the string into the char array.
  while(getline(cin,str2) && str2.length() != 0){
    str +=  (str2 + "\n");
    strcpy(ch, str.c_str());
  }

  size = str.length(); // find out the size of the file

  v.back()->size = size;
  v.back()->type = TYPE_FILE;
  //find out how many blocks the file needs in order to store the entire file
  int block_nr = ceil((float)size/(float)BLOCK_SIZE);
  //incase the size of the file is zero make block_nr 1 so we find a empty block for the file in the fat array
  if(block_nr == 0){
    block_nr=1;
  }
  int k = 0;
  //Find the next free block in the fat array where we can store the file
  //or a part of the file depending on the size of the file
  //Do this for each block needed to store the entire file
  for (int j = 0; j < block_nr; j++)
  {
    // Goes through the fat array to find the next free block in the array where we can store the
      for (int i = 2; i < BLOCK_SIZE/2; i++)
      {
          if (fat[i] == FAT_FREE)
          {
            if(j==0){
              v.back()->first_blk = i; // For the first part of the file set the first_blk for the file.
            }
            else if (j > 0)
            {
              fat[k] = i; //point the current block in the fat array to where the next part of the file is located in the array.
            }
            k = i;
            break;
          }
      }
      fat[k] = FAT_EOF; // set the last index of the file to FAT_EOF so the computer know that this is the end of the file;
  }

  cout << endl;
  int blkTemp = v.back()->first_blk;
  //Now when we know where the parts of the file is located in the fat array it is time to write the file into the disk
  //at the same blocks as the fat array

  for(int i=0; i<(int) strlen(ch)-1; i++)
  {
    //put the parts of char array into a string
    strTemp += ch[i];

    //When we have gone through char array the same amount of BLOCK_SIZE put the string into the disk at the given block
   if((i+1)%BLOCK_SIZE == 0){
      strcpy(ch2, strTemp.c_str()); //copy the string into a char array
      t8 = (uint8_t*) ch2; // convert the char array into uint8_t which the disk.write accepts
      disk.write(blkTemp,t8);
      blkTemp = fat[blkTemp]; // get next block location from the fat array
      strTemp= ""; // clear the string
    }
  }
  //Write the remainding of the file into the last block into the disk
  strcpy(ch2, strTemp.c_str());
  t8 = (uint8_t*) ch2;
  disk.write(blkTemp,t8);

  writeMetaData(nodes[currentNode]->block,v);
  disk.write(FAT_BLOCK, (uint8_t*)&fat);

  v = readMetaData(currentBlk);

  return 0;
}

// cat <filepath> reads the content of a file and prints it on the screen
int FS::cat(std::string filepath)
{
  std::cout << "FS::cat(" << filepath << ")\n";

  // split dirpath with '/' as divider
  std::vector<string> vec_filepath = split(filepath);

  if (filepath.find_first_of("/") != string::npos) {

    currentNode = travelNode(vec_filepath, MOVE, filepath.at(0));
    // set v to be the data where the user inputted the end of the path
    v = readMetaData(nodes[currentNode]->block);
  }

    int blkTemp;
    unsigned char *read = new unsigned char[BLOCK_SIZE];
    int index;
    bool exist = false;

    //Search through all the files in the current directory and find the file and which block the file starts at
    for(int i=0; i<v.size();i++){
        if(v[i]->file_name == vec_filepath.back()){
          if(v[i]->access_rights < READ){
            cout << "PERMISSION DENIED: no access right to read " << v[i]->file_name << endl;
            return 0;
          }
          blkTemp = (int) v[i]->first_blk;
          index = i; //save the location of the start block
          exist = true; // set exist to true so we know that we have found the right file
          break;
        }
    }
    //don't print out if the file was not found
    if(!exist){
      cout << "ERROR: No file with the name " << vec_filepath.back() << " could be found" << endl;
      return 0;
    }
    // if user inputted file is a directory instead
    if(v[index]->type == TYPE_DIR)
    {
      std::cerr << "ERROR: File " << vec_filepath.back() << " is a directory" << "exiting..."<< std::endl;
      return 0;
    }
    //print out everything on the file by printing out everything for each block the file is divided up on.
    while(blkTemp != FAT_EOF)
    {
      disk.read(blkTemp,read); //Read the data from the given block from the disk
      cout << read << endl; //prints out the data

      blkTemp = fat[blkTemp]; // sets the current block to the next block in the fat array until the position in the fat array is FAT_EOF
    }
    return 0;
}

// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
    cout << "FS::ls()\n";
    v = readMetaData(currentBlk);

    string access;

    //prints out each file in the current directory
    for(int i=0; i<v.size(); i++){
        access = getAccess(v,i);  // get the access rights formatted correctly into a string (rwx)

        cout  <<"filename " << v[i]->file_name << "\t type " << (int) v[i]->type  << "\t accessrights " << access << "\t size " << v[i]->size << "\tfirst blokc: " << v[i]->first_blk << endl;
    }
    printfat();
    return 0;

}
// cp <sourcefilepath> <destfilepath> makes an exact copy of the file
// <sourcefilepath> to a new file <destfilepath>
int
FS::cp(std::string sourcefilepath, std::string destfilepath)
{
    std::cout << "FS::cp(" << sourcefilepath << "," << destfilepath << ")\n";
    int srcBlkTemp;
    int desBlkTemp;
    unsigned char *read = new unsigned char[BLOCK_SIZE]; // used as buffer to read from disk

    // add all the words of the filepath into a vector
    std::vector<string> vec_srcFilepath = split(sourcefilepath);
    //checks if the sourcefilepath contains any '/'
    if (sourcefilepath.find_first_of("/") != string::npos) {
      currentNode = travelNode(vec_srcFilepath, MAKE, sourcefilepath.at(0));
      // set v to be the data where the user inputted the end of the path
      v = readMetaData(nodes[currentNode]->block);
    }
    // add all the words of the filepath into a vector
    std::vector<string> vec_destFilepath = split(destfilepath);
    int destCurrentNode = currentNode;
    vector<dir_entry*> destV = v;
    //checks if the destfilepath contains any '/'
    if (destfilepath.find_first_of("/") != string::npos) {
      destCurrentNode = travelNode(vec_destFilepath, MAKE, destfilepath.at(0));
      v = readMetaData(nodes[currentNode]->block);   // set v to be the data where the user inputted the end of the path
      destV = readMetaData(nodes[destCurrentNode]->block);   // set v to be the data where the user inputted the end of the path
    }

    //checks if the destfilepath already exist in the current directory
    for (int k = 0; k < destV.size(); k++) {
        if (destV[k]->file_name == vec_destFilepath.back()){
          cout << "ERROR: The destpath already exist\n";return 0;
        }
    }

    int i;
    for(i = 0; i < v.size(); i++){
      //Checks if the file exist in the current directory
      //if it does copy the data from source into the destination vector
      if(v[i]->file_name == vec_srcFilepath.back() )
      {
        if(v[i]->access_rights < READ){
          cout << "PERMISSION DENIED: no access right to read " << v[i]->file_name << endl;
          return 0;
        }
        destV.push_back(new dir_entry());

        strcpy(destV.back()->file_name, vec_destFilepath.back().c_str());
        srcBlkTemp=v[i]->first_blk;
        destV.back()->size = v[i]->size;
        destV.back()->type = v[i]->type;
        destV.back()->access_rights = v[i]->access_rights;
        break;
      }
      //Dont copy the file into the new filepath if the sourcefilepath is not a file
      else if(i >= v.size()-1){
        cout << "ERROR: The file wasn't found\n";return 0;
      }
    }

    // how many blocks of memory does v[i] occupy
    int block_nr = ceil((float)v[i]->size/(float)BLOCK_SIZE);

    int k = 0;
    int tempBlk = 0;
    //find free blocks in the fat array and allocate neccesary blocks to the file
    for (int j = 0; j < block_nr; j++)
    {
        for (int i = 2; i < BLOCK_SIZE/2; i++)
        {
            if (fat[i] == FAT_FREE)
            {
              if(j==0){
                // the first block to be first free block encountered
                destV.back()->first_blk = i;
              }
              else if (j > 0)
              {
                // set the earlier block to point to this new block
                fat[k] = i;
              }
              // k = last iteration
              k = i;
              break;
            }
        }
        // set the last block -1
        fat[k] = FAT_EOF;
    }

    desBlkTemp = destV.back()->first_blk;

    while(srcBlkTemp != FAT_EOF){
      disk.read(srcBlkTemp,read); //reads the blocks from sourcefilepath one by one
      disk.write(desBlkTemp,(uint8_t*)read); //writes the data that was read from the destinationfilepaths block
      srcBlkTemp = fat[srcBlkTemp]; // get the next block location for sourcefilepath
      desBlkTemp = fat[desBlkTemp];// get the next block location for destinationfilepaths
    }

    // set the   nodes[destCurrentNode]->entries vector into what is lastest written into the destV vector
    nodes[destCurrentNode]->entries.push_back(destV.back());
    writeMetaData(nodes[destCurrentNode]->block, destV); //write the data from the new file into the disk at the new free block
    disk.write(FAT_BLOCK, (uint8_t*)&fat); //update the new fat array to the disk.
    //writeMetaData(currentBlk, v);
    v = readMetaData(currentBlk); //new get the data from the directory we are currently on
    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int FS::mv(std::string sourcepath, std::string destpath)
{
  int destIndex;
  int srcIndex;
  std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
  // go through the vector v that contains all files
  // set srcIndex to be the location at the file we will be working with

  std::vector<string> vec_srcpath = split(sourcepath);

  currentNode = travelNode(vec_srcpath, MAKE, sourcepath.at(0));

  if (sourcepath.find_first_of("/") != string::npos) {
    // set v to be the data where the user inputted the end of the path
    v = readMetaData(nodes[currentNode]->block);
  }
  // find the index of the source file
  for (srcIndex = 0; srcIndex < v.size(); srcIndex++){
      if (v[srcIndex]->file_name == vec_srcpath.back())
      {
        if (v[srcIndex]->type == TYPE_DIR) {
          v = readMetaData(currentBlk);
          cout << "ERROR: sourcepath lead to a directory" << endl;
          return 0;
        }
        break;
      }
      else if(srcIndex >= v.size()-1){
        cout << "ERROR: The file wasn't found\n"; return 0;  }}

  std::vector<string> vec_destpath = split(destpath);
  int destCurrentNode = currentNode;
  vector<dir_entry*> destV = v;
  destCurrentNode = travelNode(vec_destpath, MOVE, destpath.at(0));

    v = readMetaData(nodes[currentNode]->block);
    // set v to be the data where the user inputted the end of the path
    destV = readMetaData(nodes[destCurrentNode]->block);

  bool rename = true;
  //checks if the destfilepath already exist in the current directory
  for (int k = 0; k < v.size(); k++) {
    if (v[k]->file_name == vec_destpath.back()){
      destIndex = k;
      // if it's a directory we won't rename the source file but move it instead
      if (v[k]->type == TYPE_DIR) {
        rename = false;
      }
      break;
    }
    else if(k >= v.size()-1)
    {
      cout << "We rename the file" << endl;
    }
  }

  int i;
  //Checks if the file exist in the current directory
  for(i = 0; i < v.size(); i++){
    //if it does copy the data from sourcefilepath into the destV vector
    if(v[i]->file_name == vec_srcpath.back() )
    {
      if (rename == false) /* move */{
        if (v[i]->access_rights < READ) {
          cout << "PERMISSION DENIED: no right to read " << v[i]->file_name << endl;
          v = readMetaData(currentBlk);
          return 0;
        }
        destV.push_back(new dir_entry());
        strcpy(destV.back()->file_name, vec_srcpath.back().c_str());
        destV.back()->first_blk = v[i]->first_blk;
        destV.back()->size = v[i]->size;
        destV.back()->type = v[i]->type;
        destV.back()->access_rights = v[i]->access_rights;
        v.erase(v.begin() + srcIndex);
        // set the   nodes[destCurrentNode]->entries vector into what is last written into the destV vector
        nodes[destCurrentNode]->entries = destV;
        nodes[currentNode]->entries = v;
        writeMetaData(nodes[currentNode]->block, v);
        writeMetaData(nodes[destCurrentNode]->block, destV); //write the data from the new file into
        v = readMetaData(currentBlk); //new get the data from the directory we are currently on

        return 0;

      }
      else if (rename == true) // rename
      {
        strcpy(v[i]->file_name, vec_destpath.back().c_str());
        writeMetaData(nodes[currentNode]->block, v);
        v = readMetaData(currentBlk);
        return 0;
      }

    }
    //Dont copy the file into the new filepath if the sourcefilepath is not a file
    else if(i >= v.size()-1){
      cout << "ERROR: The file wasn't found\n";return 0;
    }
  }
  return 0;
}

// rm <filepath> removes / deletes the file <filepath>
int FS::rm(std::string filepath)
{
    int destIndex;
    std::cout << "FS::rm(" << filepath << ")\n";
    // set destIndex to be the location at the file we will be working with
    std::vector<string> v_filepath = split(filepath);

    currentNode = travelNode(v_filepath, MAKE, filepath.at(0));

    if (filepath.find_first_of("/") != string::npos) {
      // set v to be the data where the user inputted the end of the path
      v = readMetaData(nodes[currentNode]->block);
    }


    bool exist = false;
    for (destIndex = 0; destIndex < v.size(); destIndex++){
      if (v[destIndex]->file_name == v_filepath.back()){
        exist = true;
        break;
      }
    }
      if(!exist){
        cout << "ERROR: The file wasn't found\n";return 0;}
      if((v[destIndex]->access_rights != WRITE || v[destIndex]->access_rights != (WRITE | EXECUTE)) && v[destIndex]->access_rights <= (READ | EXECUTE))
      {
        cout << "PERMISSION DENIED: no access right to write on " << v[destIndex]->file_name << endl;
        return 0;
      }
    int temp;
    int blk = v[destIndex]->first_blk;
    while(blk != FAT_EOF)
    {
      temp = blk;
      // block moves forward in the fat
      blk = fat[blk];
      // free up the old block
      fat[temp] = FAT_FREE;
    }
    v.erase(v.begin() + destIndex);
    nodes[currentNode]->entries = v;
    writeMetaData(nodes[currentNode]->block,v);
    disk.write(FAT_BLOCK, (uint8_t*)&fat);
    v = readMetaData(currentBlk);
    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int FS::append(std::string filepath1, std::string filepath2)
{

    std::cout << "FS::append(" << filepath1 << "," << filepath2 << ")\n";
    int tmpCur = 0;
    int destIndex;
    int srcIndex;
    unsigned char* read = new unsigned char[BLOCK_SIZE];
    unsigned char* read2 = new unsigned char[BLOCK_SIZE];


    std::vector<string> vec_srcpath = split(filepath1);

    currentNode = travelNode(vec_srcpath, MAKE, filepath1.at(0));
    //checksif the filepath1 has any "/".
    if (filepath1.find_first_of("/") != string::npos) {
      // set v to be the data where the user inputted the end of the path
      v = readMetaData(nodes[currentNode]->block);
    }

    std::vector<string> vec_destpath = split(filepath2);
    int destCurrentNode = currentNode;
    vector<dir_entry*> destV = v;
    //checksif the filepath2 has any "/".
    if (filepath2.find_first_of("/") != string::npos)
    {
      destCurrentNode = travelNode(vec_destpath, MAKE, filepath2.at(0));
      v = readMetaData(nodes[currentNode]->block);
      // set v to be the data where the user inputted the end of the path
      destV = readMetaData(nodes[destCurrentNode]->block);
    }

    // go through the vector v that contains all files
    // set destIndex to be the location at the file we will be working with
    {
    bool exist = false;
    bool exist2 = false;

    //check if the file we are looking for exist in given directory and save on what index in the directory the file is found
    for (srcIndex = 0; srcIndex < v.size(); srcIndex++) {
        if (v[srcIndex]->file_name == vec_srcpath.back()){
          if(v[srcIndex]->access_rights < READ){
            cout << "PERMISSION DENIED: no right to read " << v[srcIndex]->file_name << endl;
            v = readMetaData(currentBlk);
            return 0;
          }
          exist = true;
          break;
        }
    }
    //check if the file we are looking for exist in given directory and save on what index in the directory the file is found
    for (destIndex = 0; destIndex < destV.size(); destIndex++) {
        if (destV[destIndex]->file_name == vec_destpath.back()){
          if(destV[destIndex]->access_rights != (WRITE | READ)){
            cout << "PERMISSION DENIED: no right to write and read " << destV[destIndex]->file_name << endl;
            v = readMetaData(currentBlk);
            return 0;
          }
          exist2 = true;
          break;
        }
    }
    //if exist or exist2 is false meaning that either filepath1 or filepath2 was not found at the given directory give ERROR messsage.
    if(!exist || !exist2){
      cout << "ERROR: The file wasn't found\n";return 0;}
    }

  int blk = destV[destIndex]->first_blk;
  //check if sum of the two files together is over BLOCK_SIZE and in that case find the last block for filepath2
  if((destV[destIndex]->size + v[srcIndex]->size) > BLOCK_SIZE)
  {
      while (true){
          if (fat[blk] == FAT_EOF)
              break;
          else
              blk = fat[blk];
      }

      //find enough free blocks for the filepath2 neccesary to store all of filepath1's information
      int block_nr = ceil((float)v[srcIndex]->size / (float)BLOCK_SIZE);
      int k = 0;

      for (int j = 0; j < block_nr; j++)
      {
          for (int i = 2; i < BLOCK_SIZE / 2; i++)
          {
              if (fat[i] == FAT_FREE)
              {
                  if (j == 0) {
                      fat[blk] = i;
                      fat[i] = FAT_EOF;
                  }
                  else if (j > 0)
                  {
                      fat[k] = i;
                      fat[i] = FAT_EOF;
                  }
                  k = i;
                  break;
              }
          }
      }
  }

  int srcBlk = v[srcIndex]->first_blk;
  int desBlk = blk;
  //get the data from each block from filepath1 and copy it over the the new blocks for filepath2 which was allocated in precious foor loop
  while (srcBlk != FAT_EOF)
  {
      disk.read(srcBlk, read);
      disk.read(desBlk, read2);
      int sourceBlockSize = strlen((const char*)read);
      int destBlockSize = strlen((const char*)read2);
      // if the appending block is not full
      if (destV[destIndex]->size < BLOCK_SIZE)
      {
          // if the two data segments do not exceed the BLOCK_SIZE limit of a block
          if (sourceBlockSize + destBlockSize <= BLOCK_SIZE)
          {
              unsigned char* ch = (unsigned char*) malloc(1+ strlen((const char*)read) + strlen((const char*)read2));
              // copy the base block of the destination to ch
              strcpy((char*)ch, (const char*)read2);
              // add the information of the source block to ch
              //strcat((char*) ch, "\n");
              strcat((char*) ch, (const char*)read);
              // write in the now complete data to the same block that dest already used before
              disk.write(desBlk, (uint8_t*)ch);
              //stop the loop
              srcBlk = FAT_EOF;

          }
          // if the combined data would exceed the limit of one block
          else
          {
              // Add the information the next available block
              // then add the block as FAT_EOF to the dest file

              disk.write(desBlk, (uint8_t*)read);
              srcBlk = fat[srcBlk];
              desBlk = fat[desBlk];

          }
      }
      else
      {
          disk.write(desBlk, (uint8_t*)read);
          //set block value to the next block for each file
          srcBlk = fat[srcBlk];
          desBlk = fat[desBlk];
      }
  }

    destV[destIndex]->size += v[srcIndex]->size; //Add the sums togheter and update filepath2's new size

    // set the  nodes[destCurrentNode]->entries vector into what is lastest written into the destV vector
    nodes[destCurrentNode]->entries = destV;
      // set the   nodes[currentNode]->entries vector into what is latest written into the v vector
    nodes[currentNode]->entries = v;
    //write the data for each vector to the right block
    writeMetaData(nodes[currentNode]->block,v);
    writeMetaData(nodes[destCurrentNode]->block,destV);
    disk.write(FAT_BLOCK, (uint8_t*)&fat);

    //update the currentNode to
    for (size_t i = 0; i < nodes.size(); i++) {
      if (nodes[i]->block == currentBlk) {
        currentNode = i;
      }
    }
    
    v = readMetaData(currentBlk);
    return 0;
}

// mkdir <dirpath> creates a new sub-directory with the name <dirpath>
// in the current directory
int FS::mkdir(std::string dirpath)
{
  std::cout << "FS::mkdir(" << dirpath << ")\n";

  // split dirpath with '/' as divider
  std::vector<string> vec_filepath = split(dirpath);

  currentNode = travelNode(vec_filepath, MAKE, dirpath.at(0));
  // set v to be the data where the user inputted the end of the path
  v = readMetaData(nodes[currentNode]->block);

  // if filepath can be found in the vector of the current folder
  // dont create the directory
  for (size_t i = 0; i < v.size(); i++)
  {
    if (vec_filepath.back() == v[i]->file_name)
    {
      cout << "ERROR: you have tried to create a directory with the same name as an existing element" << endl;
      return 0;
    }
  }

  dir_entry *dir = new dir_entry();
  //set the new directorie to get EXECUTE , WRITE and READ rigths
  dir->access_rights = (EXECUTE | WRITE | READ);
  Node *newNode = new Node;
  std::vector<string> tempVec;
  //push back the vector tempVec and insert the values of vec_filepath[i]
  for (size_t i = 0; i < vec_filepath.size() -1; i++) {
    tempVec.push_back(vec_filepath[i]);
  }
  //get the parent to the created directory
  newNode->parent = nodes[travelNode(tempVec, MOVE, dirpath.at(0))];
  tempVec.clear();

  //set the depth of the directory to its parents depth plus one
  newNode->depth = newNode->parent->depth + 1;

  strcpy(newNode->name, vec_filepath.back().c_str());
  strcpy(dir->file_name, vec_filepath.back().c_str());
  dir->type = TYPE_DIR;

  //find the next free block in the fat array and then set the new node and directory to point to that block
  for (int i = 2; i < BLOCK_SIZE/2; i++)
  {
      if (fat[i] == FAT_FREE)
      {
          newNode->block = i;
          dir->first_blk = (uint16_t)i;
          fat[i] = FAT_EOF;
          break;
      }
  }
  //insert the values of the created directory into the gobal vector v
  v.push_back(dir);


  int blkTemp = v.back()->first_blk;
  std::vector<dir_entry*> vec;

  // create the sub folder '..' that points to parent
  dir_entry *d = new dir_entry;
  d->access_rights = (EXECUTE | WRITE | READ);
  d->type = TYPE_DIR;
  string s = "..";
  strcpy(d->file_name, s.c_str());
  d->first_blk = currentBlk;
  d->size = 0;

  // set the nodes[destCurrentNode]->entries vector into what is lastest written into the dir_entry d
  newNode->entries.push_back(d);
  vec.push_back(d); //set the latest written dir_entry into the vec vector
  //Update the node three for its children and parents
  nodes[currentNode]->children.push_back(newNode);
  nodes.push_back(newNode);
  //write the new information to the disk on the right block
  writeMetaData(blkTemp, vec);
  writeMetaData(nodes[currentNode]->block,v);

  //update the vector v to the data at block currentBlk in the disk
  v = readMetaData(currentBlk);

  disk.write(FAT_BLOCK, (uint8_t*)&fat);  //update the fat array

  return 0;
}


// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int FS::cd(std::string dirpath)
{
  std::cout << "FS::cd(" << dirpath << ")\n";
  // split dirpath with '/' as divider
  std::vector<string> vec_filepath = split(dirpath);

  // set currentnode to be the node user inputted
  currentNode = travelNode(vec_filepath, MOVE, dirpath.at(0));
  if (travelError == true) {
    return 0;
  }

  // if we move to the root first we reset the path
  if (dirpath.at(0) == '/' && dirpath.length() == 1 && path.size() > 0) {
    for(int i = 0; i < path.size(); i++)
    {
      path.pop_back();
    }
  }
  //Here we either remove the last value in the path if the value is ".." or adds the value furthest back in the path array
  //This helps us to keep track of where we are in the filesystem so we can later print out it in pwd()
  for (size_t i = 0; i < vec_filepath.size(); i++) {
    if (vec_filepath[i] == "..") {
cout << "pop" << endl;
      path.pop_back();
    }
    else
    {
      cout << "push" << endl;
      path.push_back(vec_filepath[i]);
    }
  }
  //set the our new location to the currentblk
  currentBlk = nodes[currentNode]->block;
  //get the information from that new location
  v = readMetaData(currentBlk);
  return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int FS::pwd()
{
    std::cout << "FS::pwd()\n";
    cout << "/";
    //Here we print out the whole path array
    for (size_t i = 1;i<path.size(); i++) {
      cout << path[i] << "/";
    }
    cout <<endl;
    return 0;
}

// chmod <accessrights> <filepath> changes the access rights for the
// file <filepath> to <accessrights>.
int FS::chmod(std::string accessrights, std::string filepath)
{
    std::cout << "FS::chmod(" << accessrights << "," << filepath << ")\n";

    std::vector<string> vec_filepath = split(filepath);
    currentNode = travelNode(vec_filepath, MAKE, filepath.at(0));

    if (filepath.find_first_of("/") != string::npos) {
      // set v to be the data where the user inputted the end of the path
      v = readMetaData(nodes[currentNode]->block);
    }
    int fileIndex;
    //Goes through the vector v and checks i a element in the vector is equal to the given filepath
    for ( fileIndex = 0; fileIndex < v.size(); fileIndex++) {
      if(v[fileIndex]->file_name == vec_filepath.back()){
        break;
      }
      //if the loop does not find a match, print out a ERROR message
      else if(fileIndex >= v.size()-1){
        cout <<"ERROR: no file with the name "<<vec_filepath.back()<< " could be located" << endl;
        return 0;
      }
    }
    //Here we set the acceess rigth depending on what the parameter accessrights is.
    switch (stoi(accessrights)) {
      case 0:
        v[fileIndex]->access_rights = 0x00;
          break;
      case 1:
        v[fileIndex]->access_rights = EXECUTE;
          break;
      case 2:
        v[fileIndex]->access_rights = WRITE;
          break;
      case 3:
        v[fileIndex]->access_rights = (EXECUTE | WRITE);
          break;
      case 4:
        v[fileIndex]->access_rights = READ;
          break;
      case 5:
        v[fileIndex]->access_rights = (EXECUTE | READ);
          break;
      case 6:
        v[fileIndex]->access_rights = (WRITE | READ);
          break;
      case 7:
        v[fileIndex]->access_rights = (EXECUTE | WRITE | READ);
          break;
      default:
        cout << accessrights << " is not valid input" << endl;
        break;

    }
    nodes[currentNode]->entries = v;
    writeMetaData(nodes[currentNode]->block,v);
    v = readMetaData(currentBlk);
    return 0;
}
//Here is a function that prints out the fat array (for visual aid)
int FS::printfat()
{
  for (size_t i = 0; i < 10; i++) {
    cout << "fat["<<i<<"]: " << fat[i] << endl;
  }
  return 0;
}

void FS::buildTree(Node *node, int depth)
{
  // go through every entry of the current node we are in
  for (int i = 0; i < node->entries.size(); i++) {
    if (node->entries.size() <= 1 && node->entries[i]->file_name == dirUpName) {
      return;
    }
    // if the type is a directory and does not have the name ".."
    else if (node->entries[i]->type == TYPE_DIR && node->entries[i]->file_name != dirUpName) {
      //
      Node *newNode = new Node;
      // create a newNode with the appropraite data
      strcpy(newNode->name, node->entries[i]->file_name);
      newNode->entries = readMetaData(node->entries[i]->first_blk);
      newNode->block = node->entries[i]->first_blk;
      newNode->depth = depth;
      newNode->parent = node;
      // push the new node into the global node vector
      nodes.push_back(newNode);
      //set the current nodes children to also contain the newNode
      node->children.push_back(newNode);
      // do this recursively
      buildTree(newNode, newNode->depth + 1);
    }
    else if (i >= node->entries.size() - 1 ) {
      return;
    }
  }
}
// splite the filepath into seperate words in a vector with '/' as limiter
std::vector<string> FS::split(string filepath)
{
  std::vector<string> vec_filepath;
  char s[56];
  strcpy(s, filepath.c_str());
  char* pch;
  pch = strtok(s, "/");
  while (pch != NULL) {
    vec_filepath.push_back(pch);
    pch = strtok(NULL, "/");
  }
  return vec_filepath;
}

int FS::travelNode(std::vector<string> vec_filepath, int type, char firstChar)
{
  // move to the folder with the same name as the last element of vec_filepath
  if (type == MOVE) {
    int temp = currentNode;
    // if the user put '/' as the first character we should always start in the root node
    if (firstChar == '/') {
      temp = 0;
      if (vec_filepath.empty() == true) {
        return temp;
      }
    }
    // go through all the words in the filepath
    for (size_t i = 0; i < vec_filepath.size() ; i++) {
      // goes up
      if (vec_filepath[i] == "..") {
        // v becomes the directory above it's current state
        v = readMetaData(nodes[temp]->parent->block);
        for (size_t j = 0; j < nodes.size(); j++) {
          if (nodes[temp]->parent->block == nodes[j]->block) {
            // set the tempNode to be the index of the parrent node
            temp = j;
            break;
          }
          else if(j >= nodes.size()-1)
          {
            travelError = true;
            cout << "ERROR: Directory was not found" << endl;
            return currentNode;
          }
        }
      }
      // goes down
      else
      {
        // for every child in current node
        for (size_t j = 0; j < nodes[temp]->children.size(); j++) {
          // if the name of a child is the same as the name that we are currently looking at in the filepath
          if (nodes[temp]->children[j]->name == vec_filepath[i]) {
            // set v to be the correct sub folder
            v = readMetaData(nodes[temp]->children[j]->block);
            // set tempNode to be the correct child index
            for (size_t k = 0; k < nodes.size(); k++)
            {
              if (nodes[temp]->children[j]->block == nodes[k]->block) {
                // set temp to be the node index of the correct child
                temp = k;
                break;
              }
            }
            break;
          }
          //if we go through the loop without finding a match print ERROR message
          else if(j >= nodes[temp]->children.size()-1)
          {
            travelError = true;
            cout << "ERROR: Directory was not found" << endl;
            return currentNode;
          }
        }
      }
    }
    return temp;
  }
  // move to the folder with the same name as the last element of vec_filepath
  if (type == MAKE) {
    int temp = currentNode;
    // if the user put '/' as the first character we should always start in the root node
    if (firstChar == '/') {
      temp = 0;
      if (vec_filepath.empty() == true) {
        return temp;
      }
    }
    // go through all the words in the filepath except the last
    for (size_t i = 0; i < vec_filepath.size()-1; i++) {
      // goes up
      if (vec_filepath[i] == "..") {
        v = readMetaData(nodes[temp]->parent->block);
        // v becomes the directory above it's current state
        for (size_t j = 0; j < nodes.size(); j++) {
          // set the tempNode to be the index of the parrent node
          if (nodes[temp]->parent->block == nodes[j]->block) {
            // set temp to be the parent node as well
            temp = j;
            break;
          }
          else if(j >= nodes.size()-1)
          {
            travelError = true;
            cout << "ERROR: Directory was not found" << endl;
            return currentNode;
          }
        }
      }
      // goes down
      else
      {
        // for every child in current node
        for (size_t j = 0; j < nodes[temp]->children.size(); j++) {
          if (nodes[temp]->children[j]->name == vec_filepath[i]) {
            v = readMetaData(nodes[temp]->children[j]->block);
            // set tempNode to be the correct child index
            for (size_t k = 0; k < nodes.size(); k++)
            {
              if (nodes[temp]->children[j]->block == nodes[k]->block) {
                // set temp to be the node index of the correct child
                temp = k;
                break;
              }
            }
            break;
          }
          else if(j >= nodes[temp]->children.size()-1)
          {
            travelError = true;
            cout << "ERROR: Directory was not found" << endl;
            return currentNode;
          }
        }
      }
    }
    return temp;
  }
  return -1;
}
//converts the access_rights of a file/directory into strings
//in order to show what type of access the file has when we call ls()
string FS::getAccess(std::vector<dir_entry*> vec1, int index){
  string access;
  switch (v[index]->access_rights) {
    case 0:
      access = "---";
      break;
    case 1:
      access = "--x";
      break;
    case 2:
      access = "-w-";
      break;
    case 3:
      access = "-wx";
      break;
    case 4:
      access = "r--";
      break;
    case 5:
      access = "r-x";
      break;
    case 6:
      access = "rw-";
      break;
    case 7:
      access = "rwx";
      break;
    default:
      cout << "Wrong input " << endl;
      break;

  }
  return access;
}
