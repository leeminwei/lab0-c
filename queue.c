#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct list_head list_head;

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    list_head *cur = head->next;
    while (cur != head) {
        list_head *toRemove = cur;
        cur = cur->next;
        element_t *entry = list_entry(toRemove, element_t, list);
        if (entry->value != NULL)
            free(entry->value);
        free(entry);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)  // 加上 !s 檢查，防止 strlen 報錯
        return false;

    element_t *node = malloc(sizeof(element_t));  // 改名為 node
    if (!node)
        return false;

    size_t size = strlen(s);
    node->value = malloc(size + 1);
    if (!node->value) {
        free(node);
        return false;
    }

    memcpy(node->value, s, size);
    node->value[size] = '\0';

    list_add(&node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)  // 同樣加上 !s 檢查
        return false;

    element_t *node = malloc(sizeof(element_t));  // 改名為 node
    if (!node)
        return false;

    size_t size = strlen(s);
    node->value = malloc(size + 1);
    if (!node->value) {
        free(node);
        return false;
    }

    memcpy(node->value, s, size);
    node->value[size] = '\0';

    list_add_tail(&node->list, head);
    return true;
}

element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *node = list_entry(head->next, element_t, list);
    list_del(&node->list);  // 使用內建巨集安全移除

    if (sp && node->value) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return node;
}

element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *node =
        list_entry(head->prev, element_t, list);  // 尾巴是 head->prev
    list_del(&node->list);

    if (sp && node->value) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return node;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    list_head *cur = head->next;
    int count = 0;
    while (cur != head) {
        count++;
        cur = cur->next;
    }
    return count;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || head->next == head)
        return false;
    list_head *fast = head->next;
    list_head *slow = head->next;
    // 由快慢指標找出中點
    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }
    slow->prev->next = slow->next;
    slow->next->prev = slow->prev;
    element_t *node = list_entry(slow, element_t, list);
    if (node->value != NULL)
        free(node->value);
    slow->next = NULL;
    slow->prev = NULL;
    free(node);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || head->next == head)
        return false;

    element_t *cur, *safe;
    bool last_dup = false;

    // 使用 safe
    // 巨集，因為我們會在迴圈中刪除節點，在這個macro中safe代表cur->next的節點型態為element_t
    list_for_each_entry_safe(cur, safe, head, list) {
        // match_next代表當前節點(cur)與下一個節點(safe)相同
        bool match_next =
            (&safe->list != head) && (strcmp(cur->value, safe->value) == 0);
        // 如果match_next或last_dup有其中一個是true就要刪除cur
        if (match_next || last_dup) {
            // 只要跟後面的匹配，或是跟前面的匹配（代表這一區塊都是重複項）
            list_del(&cur->list);
            q_release_element(cur);
        }

        // 更新狀態：如果這次跟後面匹配了，那下一位一定也是重複項
        last_dup = match_next;
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    list_head *cur = head->next;
    list_head *cur_next = cur->next;
    while (cur != head && cur_next != head) {
        list_move(cur, cur_next);
        cur = cur->next;
        cur_next = cur->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || head->next == head)
        return;
    list_head *prev_node = head->prev;
    list_head *cur = head;
    list_head *next_node = NULL;
    while (next_node != head) {
        next_node = cur->next;
        cur->prev = next_node;
        cur->next = prev_node;
        prev_node = cur;
        cur = next_node;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || k < 2)
        return;

    struct list_head new_head, tmp_head;
    INIT_LIST_HEAD(&new_head);
    INIT_LIST_HEAD(&tmp_head);

    int count = 0;
    struct list_head *cur, *safe;

    // 使用 safe 遍歷，因為我們會一邊走一邊把節點「搬走」
    list_for_each_safe(cur, safe, head) {
        count++;
        // 先把節點搬到暫存的 tmp_head 尾端
        list_move_tail(cur, &tmp_head);

        if (count == k) {
            // 湊滿 k 個了，反轉這串小的
            q_reverse(&tmp_head);
            // 拼接到結果佇列
            list_splice_tail_init(&tmp_head, &new_head);
            count = 0;
        }
    }

    // 如果最後剩下的不足 k 個（count > 0），tmp_head 裡面還有東西
    // 這些東西不需要反轉，直接拼接到結果佇列
    list_splice_tail(&tmp_head, &new_head);

    // 最後，把處理好的整串接回原本的 head
    list_splice(&new_head, head);
}

/* 合併兩個已排序的鏈結串列 (Helper function for q_sort) */
struct list_head *merge(struct list_head *l1,
                        struct list_head *l2,
                        bool descend)
{
    struct list_head temp;
    struct list_head *cur = &temp;
    while (l1 && l2) {
        element_t *e1 = list_entry(l1, element_t, list);
        element_t *e2 = list_entry(l2, element_t, list);
        int cmp = strcmp(e1->value, e2->value);
        if ((!descend && cmp <= 0) || (descend && cmp >= 0)) {
            cur->next = l1;
            l1 = l1->next;
        } else {
            cur->next = l2;
            l2 = l2->next;
        }
        cur = cur->next;
    }
    cur->next = l1 ? l1 : l2;
    return temp.next;
}

/* 遞迴分割並排序 (Helper function for q_sort) */
struct list_head *merge_sort(struct list_head *head, bool descend)
{
    if (!head || !head->next)
        return head;
    struct list_head *slow = head, *fast = head->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    struct list_head *mid = slow->next;
    slow->next = NULL;
    return merge(merge_sort(head, descend), merge_sort(mid, descend), descend);
}

void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    // 1. 斷開環狀，轉為單向鏈結
    head->prev->next = NULL;
    head->next = merge_sort(head->next, descend);
    // 2. 重新接回雙向環狀
    struct list_head *cur = head, *next = head->next;
    while (next) {
        next->prev = cur;
        cur = next;
        next = next->next;
    }
    cur->next = head;
    head->prev = cur;
}

int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    struct list_head *cur = head->prev->prev;
    while (cur != head) {
        element_t *cur_e = list_entry(cur, element_t, list);
        element_t *next_e = list_entry(cur->next, element_t, list);
        if (strcmp(cur_e->value, next_e->value) > 0) {
            struct list_head *tmp = cur;
            cur = cur->prev;
            list_del(tmp);
            q_release_element(list_entry(tmp, element_t, list));
        } else {
            cur = cur->prev;
        }
    }
    return q_size(head);
}

int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    struct list_head *cur = head->prev->prev;
    while (cur != head) {
        element_t *cur_e = list_entry(cur, element_t, list);
        element_t *next_e = list_entry(cur->next, element_t, list);
        if (strcmp(cur_e->value, next_e->value) < 0) {
            struct list_head *tmp = cur;
            cur = cur->prev;
            list_del(tmp);
            q_release_element(list_entry(tmp, element_t, list));
        } else {
            cur = cur->prev;
        }
    }
    return q_size(head);
}

int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;
    queue_contex_t *first = list_entry(head->next, queue_contex_t, chain);
    for (struct list_head *it = head->next->next; it != head; it = it->next) {
        queue_contex_t *other = list_entry(it, queue_contex_t, chain);
        list_splice_init(other->q, first->q);
    }
    q_sort(first->q, descend);
    return q_size(first->q);
}
