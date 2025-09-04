#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*
{"timestamp": "2025-08-27T10:15:30Z", "user_id": "u001", "response_time_ms": 120, "http_status": 200}
{"timestamp": "2025-08-27T10:16:10Z", "user_id": "u002", "response_time_ms": 250, "http_status": 404}
{"timestamp": "2025-08-27T11:05:00Z", "user_id": "u001", "response_time_ms": 150, "http_status": 200}
{"timestamp": "2025-08-27T11:20:45Z", "user_id": "u003", "response_time_ms": 300, "http_status": 500}
{"timestamp": "2025-08-27T11:21:15Z", "user_id": "u002", "response_time_ms": 180, "http_status": 200}
*/



int main(int argc, char const *argv[]) {
  // 判断命令行参数
  if (argc != 2) {  // 加上程序名, 未提供json路径
    std::cerr << "未知路径\n请使用:\t" << argv[0] << " <path>\n";
    return 0;
  }

  // 参数合法, 打开文件
  std::ifstream logfile(argv[1]);
  if (!logfile) {
    std::cerr << "打开日志文件失败" << '\n';
    return 1;
  }

  uint64_t                        totalRequest      = 0;  // 用于统计总请求数
  uint64_t                        totalResponseTime = 0;  // 用于统计总响应时间
  std::map<std::string, uint64_t> mapStatusCode;          // 状态码到数量的映射
  uint64_t                        hourCount[24] = {0};    // 每小时请求数

  std::string line;
  while (std::getline(logfile, line)) {  // 从文件读取每一行

    if (line.empty()) continue;  // 行为空

    json lineJson = json::parse(line);

    // 1. 统计请求数量与响应时间
    ++totalRequest;
    totalResponseTime += lineJson["response_time_ms"].get<uint64_t>();

    // 2. 状态码统计
    int status = lineJson["http_status"].get<int>();
    mapStatusCode[std::to_string(status)]++;

    // 3. 提取小时
    // 假设 timestamp 格式固定为 "YYYY-MM-DDTHH:MM:SSZ"
    // 即小时开始下标为26
    int hour = (10 * line[26] + line[27]);
    ++hourCount[hour];
  }

  // 循环结束, 文件读取完

  // 计算平均响应时间 总请求可能为0
  double avgResponseTime = totalRequest ? double(totalResponseTime) / totalRequest : 0.0;

  // 最繁忙的小时
  int busiestHour = 0;
  for (uint64_t i : hourCount) {
    if (hourCount[i] > hourCount[busiestHour]) busiestHour = i;
  }

  // 构造输出
  json analysisOut;
  analysisOut["total_requests"]           = totalRequest;
  analysisOut["average_response_time_ms"] = avgResponseTime;
  analysisOut["status_code_counts"]       = mapStatusCode;
  analysisOut["busiest_hour"]             = busiestHour;

  std::cout << analysisOut.dump(2) << '\n';

  // 输出到文件
  std::ofstream ofs("logAnalysisOut.json");
  ofs << analysisOut.dump(4);

  // test json // Json 库测试
  //   std::string str = R"({"timestamp": "2025-08-27T10:15:30Z", "user_id": "u001", "response_time_ms": 120, "http_status": 200})";

  //   json js = json::parse(str);

  //   std::cout << js["timestamp"] << '\n';

  //   json out = {
  //       {"total_requests", 1500},
  //       {"average_response_time_ms", 185.5},
  //       {"status_code_counts", {{"200", 1350}, {"404", 100}, {"500", 50}}},
  //       {"busiest_hour", 19}};

  //   std::ofstream ofs("out.json");
  //   ofs << out.dump(4);

  return 0;
}
