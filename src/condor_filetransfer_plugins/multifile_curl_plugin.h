#ifdef WIN32
#define CURL_STATICLIB // this has to match the way the curl library was built.
#endif

#include <curl/curl.h>
#include <string>

struct transfer_request {
    std::string local_file_name;
};

class FileTransferStats;

class MultiFileCurlPlugin {

  public:

    MultiFileCurlPlugin( bool diagnostic );
    ~MultiFileCurlPlugin();

    int InitializeCurl();
    int DownloadMultipleFiles( const std::string &input_filename );
    int UploadMultipleFiles( const std::string &input_filename );

    std::string GetStats() const { return _all_files_stats; }

  private:

    void InitializeStats( std::string request_url );
    void InitializeCurlHandle( const std::string &request_url );
    void FinishCurlTransfer( int rval, FILE *file );

    static size_t HeaderCallback( char* buffer, size_t size, size_t nitems, void *userdata );
    static size_t FtpWriteCallback( void* buffer, size_t size, size_t nmemb, void* stream );
    int ServerSupportsResume( const std::string &url );
    int UploadFile( const std::string &url, const std::string &local_file_name );
    int DownloadFile( const std::string &url, const std::string &local_file_name, const std::string &cred, long &partial_bytes );
    int BuildTransferRequests (const std::string & input_filename, std::vector<std::pair<std::string, transfer_request>> &requested_files) const;
    FILE *OpenLocalFile (const std::string &local_file, const char *mode) const;

    CURL* _handle{nullptr};
    std::unique_ptr<FileTransferStats> _this_file_stats{nullptr};
    bool _diagnostic{false};
    std::string _all_files_stats;
    char _error_buffer[CURL_ERROR_SIZE];

};
