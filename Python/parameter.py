
class parameter:
    value=""
    published=False
    subscribers=set()

    def __init__(self,value, pub):
        self.value=value
        self.published = pub

    def setPublished():
        self.published=True

    def setPrivate():
        self.published=False

    def getValue(self):
        
        if self.published:
            return self.value
        else:
            return "PRIVATE"

    def setValue(self,v):
        if self.published:
            self.value=v
            return "OK"
        else:
            return "PRIVATE"

    def subscribe(self,qid):
        if self.published:
            print "Published"
            self.subscribers.add( qid )
        else:
            print "Private"

    def unsubscribe(self,qid):
        if self.published:
            self.subscribers.discard( qid )
        else:
            print "Private"

    def dump(self):
        print "Value    :"+self.value

        if self.published:
            print "Published:True"
            for subscriber in self.subscribers:
                print subscriber
        else:
            print "Published:True"

    def __del__(self):
        print "Dead"

