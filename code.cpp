#include <bits/stdc++.h>
using namespace std;

struct block{
    string tag;
    int entered_time;
    int last_accessed_time;
    int valid_bit;
    int dirty_bit;
};
    // block variable type to store blocks in each line of cache

string hex_to_binary(string hex_value);
string binary_to_hex(string binary_value);

int main(){
    int time_counter = 0;
    int hits = 0;
    int misses = 0;
    string config_file,access_file;
    cout << "Enter the name of config_file and access_file: ";
    cin >> config_file >> access_file;

    srand(time(0));

    vector<string> configurations;
    // Vector that stores the configuration of the cache
    string dummy;
    ifstream file(config_file);
    while(getline(file,dummy)){
        configurations.push_back(dummy);
    }
    file.close();
    int cache_size = stoi(configurations[0]);
    int block_size = stoi(configurations[1]);
    int associativity = stoi(configurations[2]);
    string replacement_policy = configurations[3];
    string writeback_policy = configurations[4];

    vector<string> addresses;
    // Vector that stores the access sequence
    ifstream file_1(access_file);
    while(getline(file_1,dummy)){
        addresses.push_back(dummy);
    }
    file_1.close();

    int indexes = 1;
    int fully_associative = 0;
    if(associativity != 0){
        indexes = cache_size/(associativity*block_size);
    }else{
        indexes = 1;
        associativity = cache_size/block_size;
        fully_associative = 1;
    }
    int index_bits = log2(indexes);
    int offset_bits = log2(block_size);
    int tag_bits = 32-index_bits-offset_bits;
    vector<block> cache[indexes];
    // Finding the bits of different sections in address and creating a array that stores vectors where each vector stores blocks so 
    // it is like a 2d array
    for(int i = 0;i<addresses.size();i++){
            string main_address = addresses[i].substr(3);
            string add_in_binary = hex_to_binary(main_address);
            add_in_binary = add_in_binary.substr(0,32-offset_bits);
            int index;
            string index_hex;
            if(fully_associative != 1){
                index = stoi(add_in_binary.substr(tag_bits),NULL,2);
                index_hex = binary_to_hex(add_in_binary.substr(tag_bits));
            }else{
                index = 0;
                index_hex = "00";
            }
            index_hex = "0x" + index_hex;
            string tag_in_hex = add_in_binary.substr(0,tag_bits);
            tag_in_hex = binary_to_hex(tag_in_hex);
            tag_in_hex = "0x" + tag_in_hex;

            if(cache[index].size() == 0){
                if(writeback_policy == "WB" || addresses[i][0] == 'R'){
                    struct block dummy;
                    dummy.tag = tag_in_hex;
                    dummy.entered_time = time_counter;
                    dummy.last_accessed_time = time_counter;
                    dummy.valid_bit = 1;
                    dummy.dirty_bit = 0;
                    if(addresses[i][0] == 'W'){
                        dummy.dirty_bit = 1;
                    }
                    cache[index].push_back(dummy);
                }
                misses++;
                cout << "address: " << addresses[i].substr(3) << ", set: " << index_hex << ", Miss, Tag: " << tag_in_hex << endl;
                // The particular index vector does not have any blocks
            }else{
                int hit_indicator = 0;
                for(int j = 0;j<cache[index].size();j++){
                    if(cache[index][j].tag == tag_in_hex){
                        cache[index][j].last_accessed_time = time_counter;
                        hits++;
                        if(addresses[i][0] == 'W'){
                            cache[index][j].dirty_bit = 1;
                        }
                        cout << "address: " << addresses[i].substr(3) << ", set: " << index_hex << ", Hit, Tag: " << tag_in_hex << endl;   
                        hit_indicator = 1;
                    }
                }
                // The particular index vector already has the same tag(It is a Hit)
                if(hit_indicator != 1){
                    if(writeback_policy == "WB" || addresses[i][0] == 'R'){
                    if(cache[index].size() < associativity){
                            struct block dummy;
                            dummy.tag = tag_in_hex;
                            dummy.entered_time = time_counter;
                            dummy.last_accessed_time = time_counter;
                            dummy.valid_bit = 1;
                            cache[index].push_back(dummy);
                    }else{
                        int block_changed_index_lru = 0;
                        int min_time_lru = cache[index][0].last_accessed_time;
                        for(int j = 0;j<cache[index].size();j++){
                            if(cache[index][j].last_accessed_time <= min_time_lru){
                                min_time_lru = cache[index][j].last_accessed_time;
                                block_changed_index_lru = j;
                            }
                        }

                        int block_changed_index_fifo = 0;
                        int min_time_fifo = cache[index][0].entered_time;
                        for(int j = 0;j<cache[index].size();j++){
                            if(cache[index][j].entered_time <= min_time_fifo){
                                min_time_fifo = cache[index][j].entered_time;
                                block_changed_index_fifo = j;
                            }
                        }
                        if(replacement_policy == "LRU"){
                            cache[index][block_changed_index_lru].tag = tag_in_hex;
                            cache[index][block_changed_index_lru].entered_time = time_counter;
                            cache[index][block_changed_index_lru].last_accessed_time = time_counter;
                            if(addresses[i][0] == 'W'){
                                cache[index][block_changed_index_lru].dirty_bit = 0;
                            }
                        }else if(replacement_policy == "FIFO"){
                            cache[index][block_changed_index_fifo].tag = tag_in_hex;
                            cache[index][block_changed_index_fifo].entered_time = time_counter;
                            if(addresses[i][0] == 'W'){
                                cache[index][block_changed_index_fifo].dirty_bit = 0;
                            }
                        }else if(replacement_policy == "RANDOM"){
                            int random_number = rand() % associativity;
                            cache[index][random_number].tag = tag_in_hex;
                            if(addresses[i][0] == 'W'){
                                cache[index][random_number].dirty_bit = 0;
                            }
                        }
                    }
                    }
                    misses++;
                    cout << "address: " << addresses[i].substr(3) << ", set: " << index_hex << ", Miss, Tag: " << tag_in_hex << endl;
                }
            }
        time_counter++;
    }
    cout << "Total Hits: " << hits << endl;
    cout << "Total Misses: " << misses << endl;
}

string hex_to_binary(string hex_value){
    unsigned long decimal_value = stoul(hex_value,NULL,16);
    return bitset<32>(decimal_value).to_string();
}
    // This function takes a hexadecimal string and it converts it to a 32-bit binary string
string binary_to_hex(string binary_value){
    unsigned long decimal_value = stoul(binary_value,NULL,2);
    stringstream change;
    change << hex << decimal_value;
    return change.str();
}
    // This function takes a binary string and converts it to hexadecimal string