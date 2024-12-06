#include "client.h"
#include "toolbox.h"
#include <fstream>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
  // check the number of arguments
  if (argc != 13 && argc != 14)
  {
    std::cout << "./run_client partial_decoding encode_type singlestripe_placement_type multistripes_placement_type k l g_m stripe_num stage_x1 stage_x2 stage_x3 value_length" << std::endl;
    std::cout << "./run_client false Azure_LRC Optimal OPT 8 2 2 32 2 2 0 1024" << std::endl;
    exit(-1);
  }

  // define the parameters
  bool partial_decoding;
  ECProject::EncodeType encode_type;
  ECProject::SingleStripePlacementType s_placement_type;
  ECProject::MultiStripesPlacementType m_placement_type;
  int k, l, g_m, b;
  int stripe_num, value_length;
  int s_x1, s_x2, s_x3;

  // get the current working directory
  char buff[256];
  getcwd(buff, 256);
  // get the path of the executable file
  std::string cwf = std::string(argv[0]);

  // get argv[1]: partial_decoding
  partial_decoding = (std::string(argv[1]) == "true");
  // get argv[2]: encode_type
  if (std::string(argv[2]) == "Azure_LRC")
  {
    encode_type = ECProject::Azure_LRC;
  }
  else if (std::string(argv[2]) == "Optimal_Cauchy_LRC")
  {
    encode_type = ECProject::Optimal_Cauchy_LRC;
  }
  else
  {
    std::cout << "error: unknown encode_type" << std::endl;
    exit(-1);
  }
  // get argv[3]: singlestripe_placement_type
  if (std::string(argv[3]) == "Optimal")
  {
    s_placement_type = ECProject::Optimal;
  }
  else
  {
    std::cout << "error: unknown singlestripe_placement_type" << std::endl;
    exit(-1);
  }
  // get argv[4]: multistripes_placement_type
  if (std::string(argv[4]) == "Ran")
  {
    m_placement_type = ECProject::Ran;
  }
  else if (std::string(argv[4]) == "DIS")
  {
    m_placement_type = ECProject::DIS;
  }
  else if (std::string(argv[4]) == "AGG")
  {
    m_placement_type = ECProject::AGG;
  }
  else if (std::string(argv[4]) == "OPT")
  {
    m_placement_type = ECProject::OPT;
  }
  else
  {
    std::cout << "error: unknown singlestripe_placement_type" << std::endl;
    exit(-1);
  }
  // get argv[5]: k
  k = std::stoi(std::string(argv[5]));
  // get argv[6]: l
  l = std::stoi(std::string(argv[6]));
  // calculate b
  b = std::ceil((double)k / (double)l);
  // get argv[7]: g_m
  g_m = std::stoi(std::string(argv[7]));
  // get argv[8]: stripe_num
  stripe_num = std::stoi(std::string(argv[8]));
  // get argv[9]: stage_x1
  s_x1 = std::stoi(std::string(argv[9]));
  // get argv[10]: stage_x2
  s_x2 = std::stoi(std::string(argv[10]));
  // get argv[11]: stage_x3
  s_x3 = std::stoi(std::string(argv[11]));
  // get argv[12]: value_length
  value_length = std::stoi(std::string(argv[12]));

  std::string client_ip = "0.0.0.0", coordinator_ip;
  if (argc == 14)
  {
    coordinator_ip = std::string(argv[13]);
  }
  else
  {
    coordinator_ip = client_ip;
  }

  // test the connection between client and coordinator
  ECProject::Client client(client_ip, 44444, coordinator_ip + std::string(":55555"));
  std::cout << client.sayHelloToCoordinatorByGrpc("Client") << std::endl;

  int stage_num = 1;
  int x = s_x1;
  if (s_x1 == 0)
  {
    std::cout << "At least one stage of stripe merging!" << std::endl;
    exit(-1);
  }
  if (s_x2 != 0)
  {
    x *= s_x2;
    stage_num++;
  }
  if (s_x3 != 0)
  {
    x *= s_x3;
    stage_num++;
  }
  if (stripe_num > 100)
  {
    std::cout << "Do not support stripe number greater than 100!" << std::endl;
    exit(-1);
  }
  if (stripe_num % x != 0)
  {
    std::cout << "Stripe number not matches! stripe_num % (stage_x1 * stage_x2 * stage_x3) == 0 if stage_xi != 0." << std::endl;
    exit(-1);
  }

  // set the parameters stored in the variable of m_encode_parameters in coordinator
  if (client.SetParameterByGrpc({partial_decoding, encode_type, s_placement_type, m_placement_type, k, l, g_m, b, x}))
  {
    std::cout << "set parameter successfully!" << std::endl;
  }
  else
  {
    std::cout << "Failed to set parameter!" << std::endl;
  }

  std::unordered_map<std::string, std::string> key_values;

  // set
  std::cout << "[SET BEGIN]" << std::endl;
  for (int i = 0; i < stripe_num; i++)
  {
    std::string key;
    if (i < 10)
    {
      key = "Object0" + std::to_string(i);
    }
    else
    {
      key = "Object" + std::to_string(i);
    }
    std::string readpath = std::string(buff) + cwf.substr(1, cwf.rfind('/') - 1) + "/../../../data/" + key;
    // std::string readpath = "/mnt/e/erasure_codes/staged_stripe_merging/project/data/" + key;
    if (access(readpath.c_str(), 0) == -1)
    {
      std::cout << "[Client] file does not exist!" << std::endl;
      exit(-1);
    }
    // if (!std::filesystem::exists(std::filesystem::path{readpath}))
    // {
    //   std::cout << "[Read] file does not exist!" << readpath << std::endl;
    // }
    else
    {
      char *buf = new char[value_length * 1024];
      std::ifstream ifs(readpath);
      ifs.read(buf, value_length * 1024);
      client.set(key, std::string(buf));
      ifs.close();
      delete buf;
    }
  }
  std::cout << "[SET END]" << std::endl
            << std::endl;

  // get
  std::cout << "[GET BEGIN]" << std::endl;
  for (int i = 0; i < stripe_num; i++)
  {
    std::string value;
    std::string key = "Object" + std::to_string(i);
    std::string targetdir = "./client_get/";
    std::string writepath = targetdir + key;
    // if (!std::filesystem::exists(std::filesystem::path{"./client_get/"}))
    // {
    //   std::filesystem::create_directory("./client_get/");
    // }
    if (access(targetdir.c_str(), 0) == -1)
    {
      mkdir(targetdir.c_str(), S_IRWXU);
    }
    client.get(key, value);
    std::cout << "[run_client] value size " << value.size() << std::endl;
    std::ofstream ofs(writepath, std::ios::binary | std::ios::out | std::ios::trunc);
    ofs.write(value.c_str(), value.size());
    ofs.flush();
    ofs.close();
  }
  std::cout << "[GET END]" << std::endl
            << std::endl;

  // delete
  std::cout << "[DEL BEGIN]" << std::endl;
  for (int i = 0; i < stripe_num; i++)
  {
    std::string key = "Object" + std::to_string(i);
    client.delete_key(key);
  }
  std::cout << "[DEL END]" << std::endl;

  return 0;
}