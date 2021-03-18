/* 
 *
 * declare function se:qr() as xs:string external;
 * se:qr()
 * 
 * <result>base64 qr code</result>
 */


#include <boost/lexical_cast.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "QrCode.hpp"
#include "sedna_ef.h"

#include "base64.h"

using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;
using namespace cv;

static std::string printQr(const QrCode &qr);

#define DLLEXPORT

static std::string printQr(const QrCode &qr) {
	const int width = 5;
	int w = qr.getSize() * width;
	std::string out;
	Mat img(w,w,CV_8UC1);
	
	for (int y = 0; y < qr.getSize(); y++) {
		for (int x = 0; x < qr.getSize() ; x++) {
			//out += (qr.getModule(x, y) ? "##" : "  ");
			if(qr.getModule(x,y)){
				cv::rectangle(img, cv::Rect(x*width,y*width,width,width),Scalar(0,0,0),cv::FILLED);
			}else{
				cv::rectangle(img, cv::Rect(x*width,y*width,width,width),Scalar(255,255,255),cv::FILLED);
			}
		}
	}
	std::vector<BYTE> buff;
	char tmp[1024];
	try{
		cv::imencode(".jpg", img, buff);
		out = base64_encode(buff.data(),buff.size());
	}catch(const cv::Exception &ex){
		out = "false " + std::string(ex.what());
	}

	return out;
}

extern "C" {
	SEDNA_SEQUENCE_ITEM DLLEXPORT *qr(SEDNA_EF_INIT *init, SEDNA_EF_ARGS *args, char * error_msg_buf);
	void DLLEXPORT qr_init(SEDNA_EF_INIT *init, char * error_msg_buf);
	void DLLEXPORT qr_deinit(SEDNA_EF_INIT *init, char * error_msg_buf);
}

char const *ef_names[] = { "qr", NULL};
SEDNA_SEQUENCE_ITEM *item = NULL;


void qr_init(SEDNA_EF_INIT *init, char * error_msg_buf)
{
	item = (SEDNA_SEQUENCE_ITEM*)init->sedna_malloc(sizeof(SEDNA_SEQUENCE_ITEM));
	item->next = NULL;
}

void qr_deinit(SEDNA_EF_INIT *init, char * error_msg_buf)
{
	free(item->data.val_string);
}

SEDNA_SEQUENCE_ITEM *qr(SEDNA_EF_INIT *init, SEDNA_EF_ARGS *args, char * error_msg_buf)
{
	if (args->length != 1)
	{
		sprintf(error_msg_buf, "bad number of arguments!");
		return NULL;
	}
	
	SEDNA_SEQUENCE_ITEM *it = args->args[0];
	it->data.type = SEDNATYPE_string;
	std::string strInput = boost::lexical_cast<std::string>(it->data.val_string);

	if(item == NULL){
		sprintf(error_msg_buf, "item is null");
		return NULL;
	}

	// // Make and print the QR Code symbol
	const QrCode qr = QrCode::encodeText(strInput.c_str(), QrCode::Ecc::LOW);
	std::string out;
	
	// create QR Image as JPG and return image as Base64
	out = printQr(qr);

	// set return string to Sedna item
	item->data.type = SEDNATYPE_string;
	item->data.val_string = (SEDNA_string)malloc(out.length() + 1);
	strcpy(item->data.val_string, out.c_str());

	return item;
}

