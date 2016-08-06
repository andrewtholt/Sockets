
class parameter:
    value=""
    published=True
    subscribers=set()

    def __init__(self,value):
        print "Created"
        self.value=value

    def setPublished():
        self.published=True

    def setPrivate():
        self.published=False

    def getValue(self):
        return self.value

    def setValue(self,v):
        self.value=v

    def subscribe(qid):
        if self.published:
            print "Published"
            subscribers.add( qid )
        else:
            print "Private"

    def unsubscribe(qid):
        if self.published:
            subscribers.discard( qid )
        else:
            print "Private"

    def dump():
        print "Value    :"+self.value

        if self.published:
            print "Published:True"
            for subscriber in subscribers:
                print subscriber
        else:
            print "Published:True"

    def __del__(self):
        print "Dead"

