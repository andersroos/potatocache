from potatocache import cache

class PotatoCache(object):

    def __init__(self):
        cache.create("/sune")
        import time
        #time.sleep(10)
    
