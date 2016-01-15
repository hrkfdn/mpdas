// UPnP support for mpdas

#include "mpdas.h"

#include "upnpav.h"

#include "libupnpp/upnpplib.hxx"
#include "libupnpp/control/discovery.hxx"
#include "libupnpp/control/mediarenderer.hxx"
#include "libupnpp/control/renderingcontrol.hxx"

using namespace std;
using namespace UPnPClient;
using namespace UPnPP;

CUPNP* UPNP = 0;

class CUPNP::Internal {
public:

	MRDH renderer;
	AVTH avt;
	OHPRH ohpr;
	OHPLH ohpl;
	OHTMH ohtm;
	Song song;
	bool gotsong;
	time_t starttime;
	bool connected;
	bool cached;
	string cururi;
	int songpos;
};

void
CUPNP::SetSong(const Song *song)
{
	m->cached = false;
	if(song && !song->getArtist().empty() && !song->getTitle().empty()) {
		m->song = *song;
		m->gotsong = true;
		//iprintf("New song: %s - %s", m->song.getArtist().c_str(),
		// m->song.getTitle().c_str());
		AudioScrobbler->SendNowPlaying(*song);
	} else {
		m->gotsong = false;
	}
	m->starttime = time(NULL);
}

void
CUPNP::CheckSubmit(int playtime)
{
	if(!m->gotsong || m->cached || m->song.getArtist().empty() ||
	   m->song.getTitle().empty() || m->song.getDuration() < 5)
		return;

	//iprintf("CheckSubmit: playtime %d, songduration %d", playtime,
	//m->song.getDuration());
	if(playtime >= 240 ||
	   playtime >= m->song.getDuration()/2) {
		Cache->AddToCache(m->song, m->starttime);
		m->cached = true;
	}
}

CUPNP::CUPNP()
{
	m = new Internal;
	m->gotsong = false;
	m->connected = false;
	m->cached = false;
	m->songpos = -1;

        Logger::getTheLog("")->setLogLevel(Logger::LLERR);
	if(Connect())
		iprintf("%s", "Connected to UPNP.");
	else
		eprintf("%s", "Could not connect to UPNP.");
}

CUPNP::~CUPNP()
{
	delete m;
}

bool
CUPNP::Connect()
{
	m->connected = false;
	UPnPDeviceDirectory *superdir = UPnPDeviceDirectory::getTheDir();
	if (superdir == 0) {
		return false;
	}

	UPnPDeviceDesc ddesc;
	if (superdir->getDevByFName(Config->getUPnPName(), ddesc)) {
		m->renderer = MRDH(new MediaRenderer(ddesc));
	} else if (superdir->getDevByUDN(Config->getUPnPName(), ddesc)) {
		m->renderer = MRDH(new MediaRenderer(ddesc));
	}

	m->avt = m->renderer->avt();
	if (!m->avt) {
		// Let's try with openhome
		m->ohpr = m->renderer->ohpr();
		m->ohpl = m->renderer->ohpl();
		m->ohtm = m->renderer->ohtm();
		if (!m->ohpr || !m->ohpl || !m->ohtm)
			return false;
	}
	return m->connected = true;
}

bool
CUPNP::isConnected()
{
	return m->connected;
}

void
CUPNP::Update()
{
	if(!m->connected) {
		iprintf("Reconnecting in 10 seconds.");
		sleep(10);
		if(Connect())
			iprintf("%s", "Reconnected!");
		else {
			eprintf("%s", "Could not reconnect.");
			return;
		}
	}

	UPnPDirObject trackmeta;
	int reltime;
	string trackuri;
	int trackduration;
	if (m->avt) {
		UPnPClient::AVTransport::PositionInfo info;
		if (m->avt->getPositionInfo(info) != 0) {
			m->connected = false;
			return;
		}
		trackmeta = info.trackmeta;
		reltime = info.reltime;
		trackuri = info.trackuri;
		trackduration = info.trackduration;
	} else if (m->ohpr && m->ohpl && m->ohtm) {
		vector<OHProduct::Source> sources;
		int index;
		if (m->ohpr->getSources(sources) != 0 ||
			    m->ohpr->sourceIndex(&index) != 0) {
			eprintf("OpenHome: can't get source info !");
			return;
		}
		// 
		if (index < 0 || index > int(sources.size()) ||
		    sources[index].type.compare("Playlist")) {
		    // Can't use source
		    return;
		}

		int id;
		if (m->ohpl->id(&id) != 0 || id == 0) {
			return;
		}
		if (m->ohpl->read(id, &trackuri, &trackmeta) != 0) {
			return;
		}
		UPnPClient::OHTime::Time tm;
		if (m->ohtm->time(tm) != 0) {
			return;
		}
		trackduration = tm.duration;
		reltime = tm.seconds;
	} else {
		// ??
		m->connected = false;
		return;
	}

	// new song? Pb: the Uri changes slightly before the metadata, so
	// that if we use an uri change to update the meta, we get the one
	// from the previous song. So have to use the meta itself no big deal
	// In addition, duration also changes before the title ! Best to wait
	// a few secs play time before we do anything
	string artist(trackmeta.getprop("upnp:artist"));
	string album(trackmeta.getprop("upnp:album"));
	//iprintf("Current info: duration %d reltime %d, title %s",
	//	   trackduration, reltime, trackmeta.m_title.c_str());
	if(reltime >= 3 &&
	   (!m->gotsong || 
		m->song.getTitle().compare(trackmeta.m_title) ||
		m->song.getAlbum().compare(album) ||
		m->song.getArtist().compare(artist))) {

		m->cururi = trackuri;
		m->songpos = reltime;
		Song s(artist,
			   trackmeta.m_title,
			   album,
			   trackduration);
		SetSong(&s);
	}

	// song playing
	if(m->songpos != reltime) {
		m->songpos = reltime;
		CheckSubmit(reltime);
	}
}
